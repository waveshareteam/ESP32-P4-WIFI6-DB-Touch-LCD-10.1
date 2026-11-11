# ES8389 I2S Loopback

[中文](README_CN.md)

Configures the ES8389 at 7-bit I2C address `0x10` and forwards microphone audio to the speaker using 48 kHz, 16-bit stereo I2S. The sketch uses its local `ES8389` driver and does not configure MCLK. Keep the speaker volume low to reduce acoustic feedback.

## Usage

Select `Waveshare ESP32-P4-WIFI6-DB Touch LCD 10.1`, open `i2s.ino`, compile and upload it, and open the serial monitor at 115200 baud. See the [Arduino guide](../README.md) for setup and commands.
