# GPIO Input/Output Monitor

[中文版本](./README_CN.md)

This example uses LVGL to show the live level of selected GPIOs in a responsive grid.

The example does not depend on the BSP. It drives the JD9365 panel directly through `esp_lcd_jd9365_10_1` and the GT911 touch controller through `esp_lcd_touch_gt911`, then registers both with LVGL via `esp_lvgl_adapter`. The expansion-header GPIO list is defined in `s_header_gpios[]` in `main/main.c`. The top `lv_switch` changes all monitored GPIOs between `GPIO_MODE_INPUT` and `GPIO_MODE_OUTPUT`; output-mode cards can be clicked to toggle their output level.

Call `gpio_monitor_init()` before starting the display, then call `gpio_monitor_start_ui()` (while holding `esp_lv_adapter_lock()`) after the display is ready. This lets the display init reclaim any pins it uses. The grid calculates its columns and card sizes from the GPIO count and the active display resolution. If the list is larger than the screen, the grid can be scrolled vertically.

The example currently starts in `GPIO_MODE_INPUT`. Confirm that the monitored pins are not being used by the LCD, touch controller, camera, or another peripheral on the target board before switching to output mode.

## Build and Flash

```bash
cd examples/esp_idf/gpio_monitor
idf.py set-target esp32p4
idf.py build
idf.py -p PORT flash monitor
```

Replace `PORT` with the serial port.
