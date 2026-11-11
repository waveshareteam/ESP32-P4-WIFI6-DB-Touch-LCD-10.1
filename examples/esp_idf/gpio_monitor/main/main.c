/*
 * SPDX-FileCopyrightText: 2026 Waveshare
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>

#include "driver/i2c_master.h"
#include "driver/ledc.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_ldo_regulator.h"
#include "esp_lcd_jd9365_10_1.h"
#include "esp_lcd_touch_gt911.h"
#include "esp_lcd_mipi_dsi.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lv_adapter.h"
#include "lvgl.h"

#include "gpio_monitor.h"

#define LCD_H_RES                       (800)
#define LCD_V_RES                       (1280)
#define LCD_BITS_PER_PIXEL              (16)
#define LCD_MIPI_DSI_LANE_NUM           (2)
#define LCD_ROTATION                    ESP_LV_ADAPTER_ROTATE_0
#define LCD_TEAR_AVOID_MODE             ESP_LV_ADAPTER_TEAR_AVOID_MODE_TRIPLE_PARTIAL

#define LCD_PIN_NUM_RST                 (31)
#define LCD_PIN_NUM_BK_LIGHT            (30)

// Shared by the JD9365 driver (via i2c_bus) and the GT911 touch controller
#define I2C_PORT_NUM                    (1)
#define I2C_PIN_NUM_SDA                 (7)
#define I2C_PIN_NUM_SCL                 (8)

#define TOUCH_PIN_NUM_RST               (23)
#define TOUCH_PIN_NUM_INT               (52)

// Backlight PWM, higher duty means brighter
#define LCD_BK_LIGHT_LEDC_MODE          LEDC_LOW_SPEED_MODE
#define LCD_BK_LIGHT_LEDC_TIMER         LEDC_TIMER_1
#define LCD_BK_LIGHT_LEDC_CHANNEL       LEDC_CHANNEL_1
#define LCD_BK_LIGHT_LEDC_DUTY_RES      LEDC_TIMER_10_BIT
#define LCD_BK_LIGHT_LEDC_FREQ_HZ       (5000)

// MIPI DSI PHY is powered by the internal LDO channel 3
#define MIPI_DSI_PHY_PWR_LDO_CHAN       (3)
#define MIPI_DSI_PHY_PWR_LDO_VOLTAGE_MV (2500)

#define GPIO_MONITOR_NAME_LEN 8

// GPIOs routed to the expansion header
static const gpio_num_t s_header_gpios[] = {
    GPIO_NUM_32,
    GPIO_NUM_33,
    GPIO_NUM_49,
    GPIO_NUM_50,
    GPIO_NUM_51,
    GPIO_NUM_36,
    GPIO_NUM_5,
    GPIO_NUM_4,
    GPIO_NUM_3,
    GPIO_NUM_2,
    GPIO_NUM_47,
    GPIO_NUM_46,
    GPIO_NUM_48,
};

#define HEADER_GPIO_COUNT (sizeof(s_header_gpios) / sizeof(s_header_gpios[0]))

static gpio_monitor_pin_config_t s_gpio_pins[HEADER_GPIO_COUNT];
static char s_gpio_pin_names[HEADER_GPIO_COUNT][GPIO_MONITOR_NAME_LEN];
static esp_ldo_channel_handle_t s_dsi_ldo;
static i2c_master_bus_handle_t s_i2c_bus;

static const char *TAG = "gpio_monitor_main";

static esp_err_t gpio_monitor_init_from_header(void)
{
    for (size_t i = 0; i < HEADER_GPIO_COUNT; i++) {
        snprintf(s_gpio_pin_names[i], sizeof(s_gpio_pin_names[i]), "GPIO%d", s_header_gpios[i]);
        s_gpio_pins[i] = (gpio_monitor_pin_config_t) {
            .name = s_gpio_pin_names[i],
            .gpio_num = s_header_gpios[i],
            .pull_up_en = GPIO_PULLUP_DISABLE,
            .pull_down_en = GPIO_PULLDOWN_ENABLE,
            .initial_level = 0,
        };
    }

    const gpio_monitor_config_t config = {
        .pins = s_gpio_pins,
        .pin_count = HEADER_GPIO_COUNT,
        .initial_mode = GPIO_MODE_INPUT,
        .refresh_period_ms = 100,
    };
    return gpio_monitor_init(&config);
}

static void lcd_backlight_init(void)
{
    const ledc_timer_config_t timer_config = {
        .speed_mode = LCD_BK_LIGHT_LEDC_MODE,
        .duty_resolution = LCD_BK_LIGHT_LEDC_DUTY_RES,
        .timer_num = LCD_BK_LIGHT_LEDC_TIMER,
        .freq_hz = LCD_BK_LIGHT_LEDC_FREQ_HZ,
        .clk_cfg = LEDC_AUTO_CLK,
    };
    ESP_ERROR_CHECK(ledc_timer_config(&timer_config));

    const ledc_channel_config_t channel_config = {
        .gpio_num = LCD_PIN_NUM_BK_LIGHT,
        .speed_mode = LCD_BK_LIGHT_LEDC_MODE,
        .channel = LCD_BK_LIGHT_LEDC_CHANNEL,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LCD_BK_LIGHT_LEDC_TIMER,
        .duty = (1U << LCD_BK_LIGHT_LEDC_DUTY_RES) - 1,
        .hpoint = 0,
    };
    ESP_ERROR_CHECK(ledc_channel_config(&channel_config));
}

static void i2c_bus_init(void)
{
    const i2c_master_bus_config_t i2c_config = {
        .i2c_port = I2C_PORT_NUM,
        .sda_io_num = I2C_PIN_NUM_SDA,
        .scl_io_num = I2C_PIN_NUM_SCL,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };
    ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_config, &s_i2c_bus));
}

static esp_err_t touch_init(esp_lcd_touch_handle_t *ret_touch)
{
    // GT911 latches 0x5D or 0x14 depending on the INT level during reset, so try both
    static const uint8_t gt911_addresses[] = {
        ESP_LCD_TOUCH_IO_I2C_GT911_ADDRESS,
        ESP_LCD_TOUCH_IO_I2C_GT911_ADDRESS_BACKUP,
    };

    esp_err_t ret = ESP_ERR_NOT_FOUND;
    for (size_t i = 0; i < sizeof(gt911_addresses) / sizeof(gt911_addresses[0]); i++) {
        esp_lcd_panel_io_i2c_config_t io_config = ESP_LCD_TOUCH_IO_I2C_GT911_CONFIG();
        io_config.dev_addr = gt911_addresses[i];
        io_config.scl_speed_hz = 400000;
        esp_lcd_panel_io_handle_t io = NULL;
        ret = esp_lcd_new_panel_io_i2c(s_i2c_bus, &io_config, &io);
        if (ret != ESP_OK) {
            continue;
        }

        esp_lcd_touch_io_gt911_config_t gt911_config = {
            .dev_addr = io_config.dev_addr,
        };
        const esp_lcd_touch_config_t touch_config = {
            .x_max = LCD_H_RES,
            .y_max = LCD_V_RES,
            .rst_gpio_num = TOUCH_PIN_NUM_RST,
            .int_gpio_num = TOUCH_PIN_NUM_INT,
            .levels = {
                .reset = 0,
                .interrupt = 0,
            },
            .driver_data = &gt911_config,
        };
        ret = esp_lcd_touch_new_i2c_gt911(io, &touch_config, ret_touch);
        if (ret == ESP_OK) {
            ESP_LOGI(TAG, "GT911 detected at 0x%02X", io_config.dev_addr);
            return ESP_OK;
        }
        esp_lcd_panel_io_del(io);
    }
    return ret;
}

static lv_display_t *lcd_display_start(void)
{
    const esp_ldo_channel_config_t ldo_config = {
        .chan_id = MIPI_DSI_PHY_PWR_LDO_CHAN,
        .voltage_mv = MIPI_DSI_PHY_PWR_LDO_VOLTAGE_MV,
    };
    ESP_ERROR_CHECK(esp_ldo_acquire_channel(&ldo_config, &s_dsi_ldo));

    const esp_lcd_dsi_bus_config_t bus_config = JD9365_PANEL_BUS_DSI_2CH_CONFIG();
    esp_lcd_dsi_bus_handle_t dsi_bus = NULL;
    ESP_ERROR_CHECK(esp_lcd_new_dsi_bus(&bus_config, &dsi_bus));

    const esp_lcd_dbi_io_config_t io_config = JD9365_PANEL_IO_DBI_CONFIG();
    esp_lcd_panel_io_handle_t io = NULL;
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_dbi(dsi_bus, &io_config, &io));

    esp_lcd_dpi_panel_config_t dpi_config = JD9365_800_1280_PANEL_60HZ_DPI_CONFIG(LCD_COLOR_PIXEL_FORMAT_RGB565);
    dpi_config.num_fbs = esp_lv_adapter_get_required_frame_buffer_count(LCD_TEAR_AVOID_MODE, LCD_ROTATION);
    jd9365_vendor_config_t vendor_config = {
        .flags = {
            .use_mipi_interface = 1,
        },
        .mipi_config = {
            .dsi_bus = dsi_bus,
            .dpi_config = &dpi_config,
            .lane_num = LCD_MIPI_DSI_LANE_NUM,
        },
    };
    const esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = LCD_PIN_NUM_RST,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB,
        .bits_per_pixel = LCD_BITS_PER_PIXEL,
        .vendor_config = &vendor_config,
    };
    esp_lcd_panel_handle_t panel = NULL;
    ESP_ERROR_CHECK(esp_lcd_new_panel_jd9365(io, &panel_config, &panel));
    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel));
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel, true));

    const esp_lv_adapter_config_t adapter_config = ESP_LV_ADAPTER_DEFAULT_CONFIG();
    ESP_ERROR_CHECK(esp_lv_adapter_init(&adapter_config));
    esp_lv_adapter_display_config_t display_config = ESP_LV_ADAPTER_DISPLAY_MIPI_DEFAULT_CONFIG(
                panel, io, LCD_H_RES, LCD_V_RES, LCD_ROTATION);
    display_config.tear_avoid_mode = LCD_TEAR_AVOID_MODE;
    lv_display_t *display = esp_lv_adapter_register_display(&display_config);
    ESP_ERROR_CHECK(display ? ESP_OK : ESP_FAIL);

    esp_lcd_touch_handle_t touch = NULL;
    if (touch_init(&touch) == ESP_OK) {
        const esp_lv_adapter_touch_config_t touch_config = ESP_LV_ADAPTER_TOUCH_DEFAULT_CONFIG(display, touch);
        ESP_ERROR_CHECK(esp_lv_adapter_register_touch(&touch_config) ? ESP_OK : ESP_FAIL);
    } else {
        ESP_LOGW(TAG, "GT911 not found, continuing without touch");
    }

    ESP_ERROR_CHECK(esp_lv_adapter_start());
    return display;
}

void app_main(void)
{
    /* Configure GPIOs first; the display init may reclaim pins it uses. */
    ESP_ERROR_CHECK(gpio_monitor_init_from_header());
    /* Create the I2C bus before the panel so the JD9365 driver reuses it. */
    i2c_bus_init();
    lv_display_t *display = lcd_display_start();
    lcd_backlight_init();

    ESP_ERROR_CHECK(esp_lv_adapter_lock(-1));
    lv_display_set_default(display);
    ESP_ERROR_CHECK(gpio_monitor_start_ui());
    esp_lv_adapter_unlock();
}
