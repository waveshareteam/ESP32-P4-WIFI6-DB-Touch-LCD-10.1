# RTC

[中文](README_CN.md)

Reads the PCF85063A at I2C address `0x51` and prints the time every second. Invalid time or a stopped oscillator triggers initialization from the firmware compile time. Set `rtc_set_time_on_boot` to `true` to force initialization, then restore it to `false` after use.

## Usage

Select `Waveshare ESP32-P4-WIFI6-DB Touch LCD 10.1`, open `rtc.ino`, compile and upload it, and open the serial monitor at 115200 baud. See the [Arduino guide](../README.md) for setup and commands.
