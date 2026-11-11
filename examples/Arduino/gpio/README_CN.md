# GPIO 控制台

[English](README.md)

通过 Arduino-ESP32 的 `Console` 库，在串口监视器中配置和测试 GPIO。将监视器设置为 115200 波特率并选择换行结束符，然后输入 `help`。写入前先配置引脚；仅使用已确认连接的引脚，避免占用板载外设使用的引脚。

```text
gpio mode 2 out
gpio write 2 0
gpio read 2
```

## 使用方法

选择 `Waveshare ESP32-P4-WIFI6-DB Touch LCD 10.1`，打开 `gpio.ino`，编译并上传，然后以 115200 波特率打开串口监视器。环境配置见 [Arduino 指南](../README_CN.md)。
