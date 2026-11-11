# GPIO Console

[中文](README_CN.md)

Use the Arduino-ESP32 `Console` library to configure and test GPIOs through the serial monitor. Set the monitor to 115200 baud with a newline ending, then enter `help`. Configure a pin before writing it. Use only pins whose connections are known; avoid pins used by onboard peripherals.

```text
gpio mode 2 out
gpio write 2 0
gpio read 2
```

## Usage

Select `Waveshare ESP32-P4-WIFI6-DB Touch LCD 10.1`, open `gpio.ino`, compile and upload it, and open the serial monitor at 115200 baud. See the [Arduino guide](../README.md) for setup.
