/*
 * SPDX-FileCopyrightText: 2015-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize an esp_board_manager device, remembering its name if that fails.
 *
 * @param[in] name Board Manager device name; must stay valid for the life of the program.
 *
 * @return The result of esp_board_manager_init_device_by_name().
 */
esp_err_t board_diag_init_device(const char *name);

/**
 * @brief Remember the device behind a failure that board_diag_init_device() cannot see.
 *
 * @param[in] name Board Manager device name; must stay valid for the life of the program.
 */
void board_diag_set_failed_device(const char *name);

/**
 * @brief Get the most recently failed device, for reporting a board_* call that just failed.
 *
 * @return The device name, or NULL if no device failure was recorded.
 */
const char *board_diag_failed_device(void);

/**
 * @brief Name an error code from esp_board_manager or ESP-IDF.
 *
 * esp_err_to_name() does not know the Board Manager codes, and
 * esp_board_manager_err_to_name() does not know the ESP-IDF ones.
 *
 * @param[in] err Error code.
 *
 * @return The error name.
 */
const char *board_diag_err_to_name(esp_err_t err);

#ifdef __cplusplus
}
#endif
