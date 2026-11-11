#pragma once

#include <stddef.h>

#include "esp_err.h"
#include "esp_lcd_types.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t lcd_panel_init(esp_lcd_panel_handle_t *ret_panel, size_t num_frame_buffers);

#ifdef __cplusplus
}
#endif
