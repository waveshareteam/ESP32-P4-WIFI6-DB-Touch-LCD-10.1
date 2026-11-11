/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Project-owned Waveshare ESP32-P4-WIFI6-DB 10.1 LCD setup. The exact
 * JD9365 vendor driver is kept in this sketch so it does not pull in
 * ESP32_Display_Panel and its legacy I2C driver.
 */

#include <Arduino.h>

#include "lcd_panel.h"

#include "esp_lcd_jd9365_10_1.h"
#include "esp_lcd_mipi_dsi.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_ldo_regulator.h"

namespace {

constexpr int dsi_phy_ldo_channel = 3;
constexpr int dsi_phy_ldo_voltage_mv = 2500;
constexpr int dsi_lane_count = 2;

static esp_ldo_channel_handle_t dsi_phy_ldo = nullptr;
static esp_lcd_dsi_bus_handle_t dsi_bus = nullptr;
static esp_lcd_panel_io_handle_t dbi_io = nullptr;
static esp_lcd_panel_handle_t lcd_panel = nullptr;

static void set_backlight(bool enabled)
{
    pinMode(LCD_BL, OUTPUT);
    digitalWrite(LCD_BL, enabled ? LCD_BL_ON_LEVEL : LCD_BL_OFF_LEVEL);
}

static void cleanup_lcd()
{
    set_backlight(false);

    if (lcd_panel != nullptr) {
        esp_lcd_panel_del(lcd_panel);
        lcd_panel = nullptr;
    }
    if (dbi_io != nullptr) {
        esp_lcd_panel_io_del(dbi_io);
        dbi_io = nullptr;
    }
    if (dsi_bus != nullptr) {
        esp_lcd_del_dsi_bus(dsi_bus);
        dsi_bus = nullptr;
    }
    if (dsi_phy_ldo != nullptr) {
        esp_ldo_release_channel(dsi_phy_ldo);
        dsi_phy_ldo = nullptr;
    }
}

static esp_err_t report_error(const char *operation, esp_err_t err)
{
    if (err != ESP_OK) {
        Serial.printf("LCD: %s failed: %s\n", operation, esp_err_to_name(err));
    }
    return err;
}

} // namespace

extern "C" esp_err_t lcd_panel_init(esp_lcd_panel_handle_t *ret_panel, size_t num_frame_buffers)
{
    if ((ret_panel == nullptr) || (num_frame_buffers == 0)) {
        return ESP_ERR_INVALID_ARG;
    }
    *ret_panel = nullptr;

    set_backlight(false);

    const esp_ldo_channel_config_t ldo_config = {
        .chan_id = dsi_phy_ldo_channel,
        .voltage_mv = dsi_phy_ldo_voltage_mv,
    };
    esp_err_t err = esp_ldo_acquire_channel(&ldo_config, &dsi_phy_ldo);
    if (err != ESP_OK) {
        return report_error("MIPI-DSI PHY LDO initialization", err);
    }

    const esp_lcd_dsi_bus_config_t dsi_bus_config = JD9365_PANEL_BUS_DSI_2CH_CONFIG();
    err = esp_lcd_new_dsi_bus(&dsi_bus_config, &dsi_bus);
    if (err != ESP_OK) {
        cleanup_lcd();
        return report_error("MIPI-DSI bus initialization", err);
    }

    const esp_lcd_dbi_io_config_t dbi_config = JD9365_PANEL_IO_DBI_CONFIG();
    err = esp_lcd_new_panel_io_dbi(dsi_bus, &dbi_config, &dbi_io);
    if (err != ESP_OK) {
        cleanup_lcd();
        return report_error("MIPI-DSI DBI initialization", err);
    }

    esp_lcd_dpi_panel_config_t dpi_config = JD9365_800_1280_PANEL_60HZ_DPI_CONFIG(LCD_COLOR_PIXEL_FORMAT_RGB565);
    dpi_config.num_fbs = num_frame_buffers;

    jd9365_vendor_config_t vendor_config = {};
    vendor_config.flags.use_mipi_interface = 1;
    vendor_config.mipi_config.dsi_bus = dsi_bus;
    vendor_config.mipi_config.dpi_config = &dpi_config;
    vendor_config.mipi_config.lane_num = dsi_lane_count;

    const esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = LCD_RST,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB,
        .bits_per_pixel = 16,
        .flags =
            {
                .reset_active_high = LCD_RST_ACTIVE_HIGH,
            },
        .vendor_config = &vendor_config,
    };
    err = esp_lcd_new_panel_jd9365(dbi_io, &panel_config, &lcd_panel);
    if (err != ESP_OK) {
        cleanup_lcd();
        return report_error("JD9365 panel creation", err);
    }

    err = esp_lcd_panel_reset(lcd_panel);
    if (err != ESP_OK) {
        cleanup_lcd();
        return report_error("JD9365 panel reset", err);
    }

    err = esp_lcd_panel_init(lcd_panel);
    if (err != ESP_OK) {
        cleanup_lcd();
        return report_error("JD9365 panel initialization", err);
    }

    err = esp_lcd_panel_disp_on_off(lcd_panel, true);
    if (err != ESP_OK) {
        cleanup_lcd();
        return report_error("JD9365 display-on command", err);
    }

    set_backlight(true);
    *ret_panel = lcd_panel;
    Serial.println("LCD: JD9365 800x1280 MIPI-DSI initialization complete");
    return ESP_OK;
}
