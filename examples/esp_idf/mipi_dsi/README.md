# Display Color Bar

[中文版本](./README_CN.md)

Display color bars on the 10.1-inch 800 x 1280 JD9365 MIPI-DSI panel of the
ESP32-P4-WIFI6-DB-Touch-LCD-10.1. This is the simplest display bring-up example
in the repository.

## Difficulty

Intermediate.

## Hardware Required

- ESP32-P4-WIFI6-DB-Touch-LCD-10.1.

The display is brought up directly with the
[`waveshare/esp_lcd_jd9365_10_1`](https://components.espressif.com/components/waveshare/esp_lcd_jd9365_10_1)
panel driver and the ESP-IDF `esp_lcd` APIs, without a BSP or board manager.
The application does not create an LVGL UI; it only initializes the panel and
asks the DPI panel driver to generate a hardware vertical color-bar pattern.

| Signal | Configuration |
| --- | --- |
| MIPI-DSI | 2 data lanes, 1500 Mbps per lane, RGB565 |
| MIPI PHY power | Internal LDO channel 3, 2500 mV |
| LCD reset | GPIO31 |
| Backlight | LEDC PWM on GPIO30, 5 kHz, 10-bit, 100% by default |

The pins and timing are defined at the top of
[`main/test_esp_lcd_jd9365.c`](main/test_esp_lcd_jd9365.c).

## Build and Flash

```bash
cd examples/esp_idf/mipi_dsi
idf.py set-target esp32p4
idf.py build
idf.py -p COMx flash monitor
```

Replace `COMx` with the serial port.

## Expected Behavior

The serial log should show:

```text
Initialize LCD device
Display jd9365: 800x1280, 16 bpp
Show color bar pattern drawn by hardware
```

The LCD should turn on its backlight and show vertical color bars.

## Troubleshooting

- Confirm PSRAM is enabled; the DPI frame buffer is allocated from PSRAM.
- Check the backlight if the logs look correct but the screen is dark; adjust
  `LCD_BK_LIGHT_DEFAULT_PERCENT` to change the brightness.
- Verify the reset pin, LDO channel, and video timing in
  `main/test_esp_lcd_jd9365.c`.
