# RS232 桥接

[English](README.md)

在串口监视器和板载 RS232 收发器之间双向转发。监视器使用 115200 波特率，外部设备使用 9600 8N1（TX GPIO28、RX GPIO29）。

## 使用方法

选择 `Waveshare ESP32-P4-WIFI6-DB Touch LCD 10.1`，打开 `rs232.ino`，编译并上传，然后以 115200 波特率打开串口监视器。环境配置和命令见 [Arduino 指南](../../README_CN.md)。
