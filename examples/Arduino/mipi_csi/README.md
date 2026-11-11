# MIPI CSI Camera Preview

[中文](README_CN.md)

Captures OV5647 camera frames with `ESP_Video` and displays them on the 800×1280 JD9365 MIPI DSI screen. Connect the camera and display before running. The local `lcd_panel.cpp` and `esp_lcd_jd9365_10_1.c` provide the display implementation without `ESP32_Display_Panel`. The project board files use Arduino-ESP32 3.3.11 as their baseline.

`ESP32_Display_Panel` 1.0.4 uses the legacy I2C driver, while the camera uses the newer driver; linking both can trigger an I2C driver conflict. This example therefore uses the local DSI driver. Camera probing may log failures for other sensor types before detecting OV5647.

## Usage

Select `Waveshare ESP32-P4-WIFI6-DB Touch LCD 10.1`, open `mipi_csi.ino`, compile and upload it, and open the serial monitor at 115200 baud. See the [Arduino guide](../README.md) for setup.
