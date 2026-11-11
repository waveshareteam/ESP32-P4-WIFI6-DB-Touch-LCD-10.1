# MIPI DSI 显示

[English](README.md)

使用 `ESP32_Display_Panel` 初始化板载 JD9365 MIPI DSI 屏幕并显示测试图案。示例配置为 800×1280 RGB565、双通道、每通道 1500 Mbps，DPI 时钟为 80 MHz。请安装 `ESP32_Display_Panel` 及其依赖。示例包含 `esp_panel_drivers_conf.h` 和 `esp_utils_conf.h`，屏幕参数见源码。

## 使用方法

选择 `Waveshare ESP32-P4-WIFI6-DB Touch LCD 10.1`，打开 `mipi_dsi.ino`，编译并上传，然后以 115200 波特率打开串口监视器。环境配置见 [Arduino 指南](../README_CN.md)。
