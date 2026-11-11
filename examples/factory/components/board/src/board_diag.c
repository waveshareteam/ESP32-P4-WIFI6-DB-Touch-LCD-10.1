/*
 * SPDX-FileCopyrightText: 2015-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <string.h>
#include "esp_board_manager_err.h"
#include "esp_board_manager_includes.h"
#include "board_diag.h"

static const char *s_failed_device;

esp_err_t board_diag_init_device(const char *name)
{
    esp_err_t ret = esp_board_manager_init_device_by_name(name);
    if (ret != ESP_OK) {
        s_failed_device = name;
    }
    return ret;
}

void board_diag_set_failed_device(const char *name)
{
    s_failed_device = name;
}

const char *board_diag_failed_device(void)
{
    return s_failed_device;
}

const char *board_diag_err_to_name(esp_err_t err)
{
    const char *name = esp_board_manager_err_to_name(err);
    return strcmp(name, "UNKNOWN_BOARD_ERROR") == 0 ? esp_err_to_name(err) : name;
}
