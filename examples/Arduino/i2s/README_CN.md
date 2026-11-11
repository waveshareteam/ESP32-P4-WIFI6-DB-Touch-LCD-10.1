# ES8389 I2S 回环

[English](README.md)

通过 7 位 I2C 地址 `0x10` 配置 ES8389，并以 48 kHz、16 位立体声 I2S 将麦克风音频送到扬声器。示例使用目录内的 `ES8389` 驱动，不配置 MCLK。请先降低扬声器音量，避免声学反馈。

## 使用方法

选择 `Waveshare ESP32-P4-WIFI6-DB Touch LCD 10.1`，打开 `i2s.ino`，编译并上传，然后以 115200 波特率打开串口监视器。环境配置和命令见 [Arduino 指南](../README_CN.md)。
