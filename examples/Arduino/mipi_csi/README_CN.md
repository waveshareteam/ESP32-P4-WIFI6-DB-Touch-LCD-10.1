# MIPI CSI 摄像头预览

[English](README.md)

通过 `ESP_Video` 采集 OV5647 摄像头画面，并显示在 800×1280 JD9365 MIPI DSI 屏幕上。运行前请连接摄像头和屏幕。本示例使用目录内的 `lcd_panel.cpp` 和 `esp_lcd_jd9365_10_1.c` 驱动屏幕，不依赖 `ESP32_Display_Panel`。项目板级文件以 Arduino-ESP32 3.3.11 为基线。

`ESP32_Display_Panel` 1.0.4 使用旧版 I2C 驱动，摄像头使用新版驱动；同时链接可能触发 I2C 驱动冲突，因此本示例使用本地 DSI 驱动。摄像头探测可能在识别 OV5647 之前输出其他传感器的探测失败日志。

## 使用方法

选择 `Waveshare ESP32-P4-WIFI6-DB Touch LCD 10.1`，打开 `mipi_csi.ino`，编译并上传，然后以 115200 波特率打开串口监视器。环境配置见 [Arduino 指南](../README_CN.md)。
