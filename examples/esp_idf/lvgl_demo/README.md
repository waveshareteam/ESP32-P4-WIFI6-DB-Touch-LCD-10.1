# LVGL Demo

[中文版本](./README_CN.md)

Runs `lv_demo_benchmark` on the 800 x 1280 JD9365 display. `esp_board_manager` initializes the panel and backlight, and `esp_lvgl_adapter` manages the LVGL display and task to run the benchmark once.

## Hardware

- ESP32-P4-WIFI6-DB-Touch-LCD-10.1.

The display is brought up by `esp_board_manager` from the board definition in
[`components/waveshare_boards/esp32_p4_wifi6_db_touch_lcd_10_1`](../../../components/waveshare_boards/esp32_p4_wifi6_db_touch_lcd_10_1/).
The application reads that description and initializes these devices:

| Board Manager device | Purpose |
| --- | --- |
| `display_lcd` | JD9365 panel on 2-lane MIPI-DSI, including the DSI bus and MIPI PHY LDO. Resolution, color depth and frame buffer count (`num_fbs`) come from here |
| `lcd_brightness` | GPIO30 backlight PWM, set to `default_percent` |

The touch panel is not used.

## Build and flash

Requires ESP-IDF >= 6.0.

```bash
cd examples/esp_idf/lvgl_demo
idf.py bmgr -b esp32_p4_wifi6_db_touch_lcd_10_1 -c ../../../components/waveshare_boards
idf.py build
idf.py -p COMx flash monitor
```

The `bmgr` command generates `components/gen_bmgr_codes` from the board
definition and selects the `esp32p4` target. Re-run it after changing the board
YAML. Board defaults (32 MB flash, HEX PSRAM) are only applied when no
`sdkconfig` exists yet, so delete a stale `sdkconfig` first. Replace `COMx` with
the actual serial port.

## Expected serial output

```text
I (...) lvgl_benchmark: LVGL benchmark started
```

The display then runs the LVGL benchmark scenes and shows the results on screen when they finish.

## Troubleshooting

- If `esp_board_manager_includes.h` or the device names cannot be found at build time, run the `idf.py bmgr` command above.
- If the log looks right but the screen stays black, check the backlight (`lcd_brightness`).
