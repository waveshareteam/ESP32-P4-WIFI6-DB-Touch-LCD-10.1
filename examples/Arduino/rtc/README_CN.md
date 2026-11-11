# RTC

[English](README.md)

读取 I2C 地址为 `0x51` 的 PCF85063A，每秒打印时间。时间无效或振荡器停止时，程序用固件编译时间初始化 RTC。将 `rtc_set_time_on_boot` 设为 `true` 可强制初始化，使用后建议恢复为 `false`。

## 使用方法

选择 `Waveshare ESP32-P4-WIFI6-DB Touch LCD 10.1`，打开 `rtc.ino`，编译并上传，然后以 115200 波特率打开串口监视器。环境配置和命令见 [Arduino 指南](../README_CN.md)。
