# BSP: ESP32-P4-WIFI6-DB-TOUCH-LCD-10.1

## Overview

This component preserves the ESP32-P4 Function EV Board BSP API while adapting
the board implementation for ESP32-P4-WIFI6-DB-TOUCH-LCD-10.1.

The display is a fixed 800x1280 JD9365 MIPI-DSI panel with GT911 touch. The SD
card uses SDMMC host slot 0. Audio uses ES8389 with MCLK GPIO13, BCLK GPIO12,
WS GPIO10, DOUT GPIO9, DIN GPIO11, and PA enable GPIO53. A PCF85063A RTC at
I2C address `0x51` shares the board I2C bus.

The ES8389 codec configuration sets `no_dac_ref = true`. For two-channel
recording, the right channel is left empty instead of being filled with the
DAC output reference.

The current component manifest version is `0.1.2`.

## Configuration

Configuration in `menuconfig`.

The BSP supports one LCD configuration, JD9365 at 800x1280, with RGB565
(default) or RGB888 selected in Kconfig. The MIPI-DPI frame-buffer count is
configurable from 1 to 3 and defaults to 3.

The expansion header GPIO list is available through
`bsp_get_header_gpios()`.

`bsp_display_brightness_set()` changes the LEDC backlight duty and
`bsp_display_brightness_get()` returns the current duty as a percentage.

`bsp_rtc_init()` preserves a valid retained RTC value. If the RTC date is
invalid or uninitialized, it sets the time once from the firmware build
timestamp. Use `bsp_rtc_set_build_time()` when an explicit overwrite is
required.

## Capabilities and dependencies

<div align="center">
<!-- START_DEPENDENCIES -->

|     Available    |       Capability       |     Controller/Codec     |                                                                                                                                                         Component                                                                                                                                                        |                            Version                           |
|------------------|------------------------|--------------------------|--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|--------------------------------------------------------------|
|:heavy_check_mark:|     :pager: DISPLAY    |JD9365|[waveshare/esp_lcd_jd9365_10_1](https://components.espressif.com/components/waveshare/esp_lcd_jd9365_10_1)|2.0.0|
|:heavy_check_mark:|:black_circle: LVGL Adapter|                      |                                                                                                      [espressif/esp_lvgl_adapter](https://components.espressif.com/components/espressif/esp_lvgl_adapter)                                                                                                      |                           ^0.6.2                             |
|:heavy_check_mark:|    :point_up: TOUCH    |           GT911          |                                                                                                        [espressif/esp_lcd_touch_gt911](https://components.espressif.com/components/espressif/esp_lcd_touch_gt911)                                                                                                        |                              1.2.0~3                         |
|        :x:       | :radio_button: BUTTONS |                          |                                                                                                                                                                                                                                                                                                                          |                                                              |
|:heavy_check_mark:|  :musical_note: AUDIO  |                          |                                                                                                              [espressif/esp_codec_dev](https://components.espressif.com/components/espressif/esp_codec_dev)                                                                                                              |                             ~1.5                             |
|:heavy_check_mark:| :speaker: AUDIO_SPEAKER|          ES8389          |                                                                                                                                                                                                                                                                                                                          |                                                              |
|:heavy_check_mark:| :microphone: AUDIO_MIC |          ES8389          |                                                                                                                                                                                                                                                                                                                          |                                                              |
|:heavy_check_mark:|  :floppy_disk: SDCARD  |                          |                                                                                                                                                            idf                                                                                                                                                           |                             >=5.4                            |
|        :x:       |    :video_game: IMU    |                          |                                                                                                                                                                                                                                                                                                                          |                                                              |
|:heavy_check_mark:|     :camera: CAMERA    |      OV5647, SC2336      |                                                                                                                  [espressif/esp_video](https://components.espressif.com/components/espressif/esp_video)                                                                                                                  |                           ^2.3.0                             |
|:heavy_check_mark:|       :clock1: RTC     |        PCF85063A         |                                                                                                           [waveshare/pcf85063a](https://components.espressif.com/components/waveshare/pcf85063a)                                                                                                           |                           ^2.0.0                             |

<!-- END_DEPENDENCIES -->
</div>

## Board-specific interfaces

### RTC

```c
ESP_ERROR_CHECK(bsp_rtc_init());

pcf85063a_datetime_t now = {0};
ESP_ERROR_CHECK(bsp_rtc_get_time(&now));
```

The BSP also provides `bsp_rtc_set_time()`, `bsp_rtc_set_build_time()`,
`bsp_rtc_deinit()`, and `bsp_rtc_get_handle()`. The returned handle is
BSP-owned and can be used with advanced PCF85063A alarm and timer APIs.

### Display brightness

```c
ESP_ERROR_CHECK(bsp_display_brightness_set(50));
int brightness = bsp_display_brightness_get();
```

The getter reports the configured LEDC duty. It is not a measurement of panel
luminance.

### Expansion header GPIOs

```c
size_t gpio_count = 0;
const gpio_num_t *gpios = bsp_get_header_gpios(&gpio_count);
```

The returned array is BSP-owned and read-only.

The interfaces above have only been checked against the source in this
workspace. Build, flash, display, audio, RTC, and electrical validation remain
separate steps.

## Compatible BSP Examples

<div align="center">
<!-- START_EXAMPLES -->

| Example | Description | Try with ESP Launchpad |
| ------- | ----------- | ---------------------- |
| [Display Example](https://github.com/espressif/esp-bsp/tree/master/examples/display) | Show an image on the screen with a simple startup animation (LVGL) | [Flash Example](https://espressif.github.io/esp-launchpad/?flashConfigURL=https://espressif.github.io/esp-bsp/config.toml&app=display-) |
| [Camera Example](https://github.com/espressif/esp-bsp/tree/master/examples/display_camera_video) | Stream camera output to display (LVGL) | [Flash Example](https://espressif.github.io/esp-launchpad/?flashConfigURL=https://espressif.github.io/esp-bsp/config.toml&app=display_camera_video) |
| [LVGL Benchmark Example](https://github.com/espressif/esp-bsp/tree/master/examples/display_lvgl_benchmark) | Run LVGL benchmark tests | - |
| [LVGL Demos Example](https://github.com/espressif/esp-bsp/tree/master/examples/display_lvgl_demos) | Run the LVGL demo player - all LVGL examples are included (LVGL) | [Flash Example](https://espressif.github.io/esp-launchpad/?flashConfigURL=https://espressif.github.io/esp-bsp/config.toml&app=display_lvgl_demos-) |
| [Display Rotation Example](https://github.com/espressif/esp-bsp/tree/master/examples/display_rotation) | Rotate screen using buttons or an accelerometer (`BSP_CAPS_IMU`, if available) | [Flash Example](https://espressif.github.io/esp-launchpad/?flashConfigURL=https://espressif.github.io/esp-bsp/config.toml&app=display_rotation-) |
| [Display SD card Example](https://github.com/espressif/esp-bsp/tree/master/examples/display_sdcard) | Example of mounting an SD card using SDMMC with display interaction. This example is also supported on boards without a display. | [Flash Example](https://espressif.github.io/esp-launchpad/?flashConfigURL=https://espressif.github.io/esp-bsp/config.toml&app=display_sdcard) |
| [USB HID Example](https://github.com/espressif/esp-bsp/tree/master/examples/display_usb_hid) | USB HID demo (keyboard, mouse, or gamepad visualization using LVGL) | - |

<!-- END_EXAMPLES -->
</div>

<!-- START_BENCHMARK -->

## LVGL Benchmark

**DATE:** 08.01.2026 01:35

**LVGL version:** 9.4.0

| Name | Avg. CPU | Avg. FPS | Avg. time | render time | flush time |
| ---- | :------: | :------: | :-------: | :---------: | :--------: |
| Empty screen | 57%  | 81  | 5  | 5  | 0  |
| Moving wallpaper | 96%  | 70  | 11  | 8  | 3  |
| Single rectangle | 30%  | 99  | 0  | 0  | 0  |
| Multiple rectangles | 43%  | 89  | 2  | 1  | 1  |
| Multiple RGB images | 33%  | 96  | 1  | 1  | 0  |
| Multiple ARGB images | 58%  | 85  | 5  | 5  | 0  |
| Rotated ARGB images | 88%  | 59  | 14  | 14  | 0  |
| Multiple labels | 99%  | 61  | 12  | 11  | 1  |
| Screen sized text | 100%  | 13  | 69  | 66  | 3  |
| Multiple arcs | 98%  | 46  | 17  | 15  | 2  |
| Containers | 33%  | 89  | 3  | 3  | 0  |
| Containers with overlay | 92%  | 28  | 31  | 30  | 1  |
| Containers with opa | 39%  | 91  | 4  | 4  | 0  |
| Containers with opa_layer | 66%  | 71  | 13  | 13  | 0  |
| Containers with scrolling | 98%  | 29  | 30  | 29  | 1  |
| Widgets demo | 99%  | 17  | 49  | 47  | 2  |
| All scenes avg. | 70%  | 64  | 15  | 15  | 0  |



<!-- END_BENCHMARK -->
