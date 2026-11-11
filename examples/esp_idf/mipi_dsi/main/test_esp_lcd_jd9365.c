/*
 * SPDX-FileCopyrightText: 2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "driver/ledc.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_check.h"
#include "esp_ldo_regulator.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_mipi_dsi.h"
#include "esp_lcd_jd9365_10_1.h"

#define LCD_H_RES                   (800)
#define LCD_V_RES                   (1280)
#define LCD_BITS_PER_PIXEL          (16)
#define LCD_DPI_PX_FORMAT           (LCD_COLOR_PIXEL_FORMAT_RGB565)
#define LCD_MIPI_DSI_LANE_NUM       (2)

#define LCD_PIN_NUM_RST             (31)
#define LCD_PIN_NUM_BK_LIGHT        (30)

// Backlight PWM, higher duty means brighter
#define LCD_BK_LIGHT_LEDC_MODE      LEDC_LOW_SPEED_MODE
#define LCD_BK_LIGHT_LEDC_TIMER     LEDC_TIMER_1
#define LCD_BK_LIGHT_LEDC_CHANNEL   LEDC_CHANNEL_1
#define LCD_BK_LIGHT_LEDC_DUTY_RES  LEDC_TIMER_10_BIT
#define LCD_BK_LIGHT_LEDC_FREQ_HZ   (5000)
#define LCD_BK_LIGHT_DEFAULT_PERCENT (100)

// MIPI DSI PHY is powered by the internal LDO channel 3
#define MIPI_DSI_PHY_PWR_LDO_CHAN       (3)
#define MIPI_DSI_PHY_PWR_LDO_VOLTAGE_MV (2500)

static char *TAG = "jd9365_test";

static esp_err_t lcd_init(esp_lcd_panel_handle_t *ret_panel)
{
    esp_ldo_channel_handle_t ldo_mipi_phy = NULL;
    esp_lcd_dsi_bus_handle_t mipi_dsi_bus = NULL;
    esp_lcd_panel_io_handle_t mipi_dbi_io = NULL;

    // Turn on the power for MIPI DSI PHY, so it can go from "No Power" state to "Shutdown" state
    ESP_LOGI(TAG, "MIPI DSI PHY powered on");
    esp_ldo_channel_config_t ldo_mipi_phy_config = {
        .chan_id = MIPI_DSI_PHY_PWR_LDO_CHAN,
        .voltage_mv = MIPI_DSI_PHY_PWR_LDO_VOLTAGE_MV,
    };
    ESP_RETURN_ON_ERROR(esp_ldo_acquire_channel(&ldo_mipi_phy_config, &ldo_mipi_phy), TAG, "Acquire LDO failed");

    ESP_LOGI(TAG, "Initialize MIPI DSI bus");
    esp_lcd_dsi_bus_config_t bus_config = JD9365_PANEL_BUS_DSI_2CH_CONFIG();
    ESP_RETURN_ON_ERROR(esp_lcd_new_dsi_bus(&bus_config, &mipi_dsi_bus), TAG, "New DSI bus failed");

    ESP_LOGI(TAG, "Install panel IO");
    esp_lcd_dbi_io_config_t dbi_config = JD9365_PANEL_IO_DBI_CONFIG();
    ESP_RETURN_ON_ERROR(esp_lcd_new_panel_io_dbi(mipi_dsi_bus, &dbi_config, &mipi_dbi_io), TAG, "New panel IO failed");

    ESP_LOGI(TAG, "Install LCD driver of jd9365");
    esp_lcd_dpi_panel_config_t dpi_config = JD9365_800_1280_PANEL_60HZ_DPI_CONFIG(LCD_DPI_PX_FORMAT);
    jd9365_vendor_config_t vendor_config = {
        .flags = {
            .use_mipi_interface = 1,
        },
        .mipi_config = {
            .dsi_bus = mipi_dsi_bus,
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
    esp_lcd_panel_handle_t panel_handle = NULL;
    ESP_RETURN_ON_ERROR(esp_lcd_new_panel_jd9365(mipi_dbi_io, &panel_config, &panel_handle), TAG, "New panel failed");
    ESP_RETURN_ON_ERROR(esp_lcd_panel_reset(panel_handle), TAG, "Panel reset failed");
    ESP_RETURN_ON_ERROR(esp_lcd_panel_init(panel_handle), TAG, "Panel init failed");
    ESP_RETURN_ON_ERROR(esp_lcd_panel_disp_on_off(panel_handle, true), TAG, "Panel display on failed");

    *ret_panel = panel_handle;
    return ESP_OK;
}

static esp_err_t lcd_brightness_init(void)
{
    const ledc_timer_config_t timer_config = {
        .speed_mode = LCD_BK_LIGHT_LEDC_MODE,
        .duty_resolution = LCD_BK_LIGHT_LEDC_DUTY_RES,
        .timer_num = LCD_BK_LIGHT_LEDC_TIMER,
        .freq_hz = LCD_BK_LIGHT_LEDC_FREQ_HZ,
        .clk_cfg = LEDC_AUTO_CLK,
    };
    ESP_RETURN_ON_ERROR(ledc_timer_config(&timer_config), TAG, "Configure backlight timer failed");

    const ledc_channel_config_t channel_config = {
        .gpio_num = LCD_PIN_NUM_BK_LIGHT,
        .speed_mode = LCD_BK_LIGHT_LEDC_MODE,
        .channel = LCD_BK_LIGHT_LEDC_CHANNEL,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LCD_BK_LIGHT_LEDC_TIMER,
        .duty = 0,
        .hpoint = 0,
    };
    return ledc_channel_config(&channel_config);
}

static esp_err_t lcd_brightness_set(int brightness_percent)
{
    brightness_percent = brightness_percent < 0 ? 0 : brightness_percent;
    brightness_percent = brightness_percent > 100 ? 100 : brightness_percent;

    const uint32_t duty_max = (1U << LCD_BK_LIGHT_LEDC_DUTY_RES) - 1;
    const uint32_t duty = duty_max * (uint32_t)brightness_percent / 100U;
    ESP_RETURN_ON_ERROR(ledc_set_duty(LCD_BK_LIGHT_LEDC_MODE, LCD_BK_LIGHT_LEDC_CHANNEL, duty), TAG, "Set backlight duty failed");
    return ledc_update_duty(LCD_BK_LIGHT_LEDC_MODE, LCD_BK_LIGHT_LEDC_CHANNEL);
}

void app_main(void)
{
    ESP_LOGI(TAG, "Initialize LCD device");
    esp_lcd_panel_handle_t panel_handle = NULL;
    esp_err_t ret = lcd_init(&panel_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize display: %s", esp_err_to_name(ret));
        return;
    }
    ESP_LOGI(TAG, "Display jd9365: %dx%d, %d bpp", LCD_H_RES, LCD_V_RES, LCD_BITS_PER_PIXEL);

    ret = lcd_brightness_init();
    if (ret == ESP_OK) {
        ret = lcd_brightness_set(LCD_BK_LIGHT_DEFAULT_PERCENT);
    }
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to turn on display backlight: %s", esp_err_to_name(ret));
    }

    ESP_LOGI(TAG, "Show color bar pattern drawn by hardware");
    ret = esp_lcd_dpi_panel_set_pattern(panel_handle, MIPI_DSI_PATTERN_BAR_VERTICAL);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to show color bar: %s", esp_err_to_name(ret));
    }
}
