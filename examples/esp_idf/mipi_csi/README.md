| Supported Targets | ESP32-P4 |
| ----------------- | -------- |

[中文版本](./README_CN.md)

# Video LCD Display

This example is based on the [esp_video](https://github.com/espressif/esp-video-components/tree/master/esp_video) component and demonstrates how to display images from the camera on an LCD screen. The application directly initializes the JD9365 display and OV5647 camera, opens the MIPI-CSI video device, uses PPA to copy each frame (center-cropped when it is larger than the display), and blits it into the LCD frame buffer through the LVGL adapter dummy-draw path. The IPA config `main/ov5647_custom.json` adds gray-world auto white balance.

## ESP-IDF Required

- This example supports ESP-IDF 5.5 and later branches.
- The project depends on `esp_video`, `esp_lvgl_adapter`, and `waveshare/esp_lcd_jd9365_10_1`.
- Please follow the [ESP-IDF Programming Guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/index.html) to set up the development environment. **We highly recommend** you [Build Your First Project](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/index.html#build-your-first-project) to get familiar with ESP-IDF and make sure the environment is set up correctly.

### Prerequisites

* ESP32-P4-WIFI6-DB-Touch-LCD-10.1 and an `esp_video` supported MIPI-CSI camera.
* USB-C cable for power and programming.

### Configure the Project

Run `idf.py menuconfig` and configure the camera sensor and video pipeline options.

The SCCB/I2C bus of the MIPI-CSI camera is created in `app_main()` of `main/main.c` (I2C port 1); change the pins there, not in menuconfig:

| Signal | GPIO |
| --- | --- |
| SCL | GPIO8 |
| SDA | GPIO7 |

In the `Espressif Camera Sensors` configuration menu, select the camera sensor that matches your hardware. The current defaults are:

```
Component config  --->
    Espressif Camera Sensors Configurations  --->
        [*] OV5647  ---->
            Default format select for MIPI  --->
                (X) RAW8 800x1280 50fps, MIPI input
```

If you use SC2336 or another sensor, update the sensor selection and output format to match the camera module.

### Build and Flash

Build the project and flash it to the board, then run monitor tool to view serial output (replace `PORT` with your board's serial port name):

```bash
idf.py set-target esp32p4
idf.py -p PORT flash monitor
```

To exit the serial monitor, type ``Ctrl-]``.

See the [ESP-IDF Getting Started Guide](https://docs.espressif.com/projects/esp-idf/en/latest/get-started/index.html) for full steps to configure and use ESP-IDF to build projects.

### Expected Behavior

The display backlight turns on and the LCD shows the live camera image. The log prints the video driver version, device name, bus information, and the detected frame width and height. Frames are allocated in PSRAM and copied into the triple LCD frame buffer by PPA. The log prints the frame rate and the PPA and flush time once per second.

### Troubleshooting

- Run [mipi_dsi](../mipi_dsi/) first to verify the LCD path.
- Check camera FPC orientation, sensor power, SCCB/I2C pins, and selected sensor model if `video cam open failed` appears.
- Confirm PSRAM is enabled and stable; camera buffers are allocated from PSRAM.
- If the image is cropped, mirrored, or rotated incorrectly, adjust the sensor output format or the PPA operation in `main/main.c`. Do not enable the OV5647 VFLIP, it changes the Bayer order and breaks the colors.
