# MIPI DSI Display

[中文](README_CN.md)

Uses `ESP32_Display_Panel` to initialize the board’s JD9365 MIPI DSI display and show test patterns. The sketch configures 800×1280 RGB565, two lanes at 1500 Mbps per lane and an 80 MHz DPI clock. Install `ESP32_Display_Panel` and its dependencies. The example includes `esp_panel_drivers_conf.h` and `esp_utils_conf.h`; see the sketch for panel parameters.

## Usage

Select `Waveshare ESP32-P4-WIFI6-DB Touch LCD 10.1`, open `mipi_dsi.ino`, compile and upload it, and open the serial monitor at 115200 baud. See the [Arduino guide](../README.md) for setup.
