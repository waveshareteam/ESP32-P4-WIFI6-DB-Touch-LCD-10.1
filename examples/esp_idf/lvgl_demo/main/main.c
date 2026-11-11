/*
 * SPDX-FileCopyrightText: 2026 Espressif Systems (Shanghai) CO LTD
 * SPDX-License-Identifier: Apache-2.0
 */

#include "esp_board_manager_includes.h"
#include "esp_lv_adapter.h"
#include "esp_log.h"
#include "demos/lv_demos.h"

#if !CONFIG_ESP_BOARD_DEV_DISPLAY_LCD_SUB_DSI_SUPPORT
#error "Board Manager DSI display support is required"
#endif

static const char *TAG = "lvgl_benchmark";

void app_main(void)
{
    ESP_ERROR_CHECK(esp_board_manager_init_device_by_name(ESP_BOARD_DEVICE_NAME_DISPLAY_LCD));

    dev_display_lcd_handles_t *lcd = NULL;
    dev_display_lcd_config_t *lcd_config = NULL;
    ESP_ERROR_CHECK(esp_board_manager_get_device_handle(ESP_BOARD_DEVICE_NAME_DISPLAY_LCD, (void **)&lcd));
    ESP_ERROR_CHECK(esp_board_manager_get_device_config(ESP_BOARD_DEVICE_NAME_DISPLAY_LCD, (void **)&lcd_config));
    ESP_ERROR_CHECK(lcd && lcd->panel_handle && lcd_config ? ESP_OK : ESP_ERR_INVALID_STATE);

    ESP_ERROR_CHECK(esp_board_manager_init_device_by_name(ESP_BOARD_DEVICE_NAME_LCD_BRIGHTNESS));

    const esp_lv_adapter_config_t adapter_config = ESP_LV_ADAPTER_DEFAULT_CONFIG();
    ESP_ERROR_CHECK(esp_lv_adapter_init(&adapter_config));
    esp_lv_adapter_display_config_t display_config = ESP_LV_ADAPTER_DISPLAY_MIPI_DEFAULT_CONFIG(
                lcd->panel_handle, lcd->io_handle, lcd_config->lcd_width, lcd_config->lcd_height,
                ESP_LV_ADAPTER_ROTATE_0);
    lv_display_t *display = esp_lv_adapter_register_display(&display_config);
    ESP_ERROR_CHECK(display ? ESP_OK : ESP_FAIL);
    ESP_ERROR_CHECK(esp_lv_adapter_start());

    ESP_ERROR_CHECK(esp_lv_adapter_lock(-1));
    lv_display_set_default(display);
    lv_demo_benchmark();
    esp_lv_adapter_unlock();
    ESP_LOGI(TAG, "LVGL benchmark started");
}
