| 支持目标 | ESP32-P4 |
| -------- | -------- |

[English Version](./README.md)

# Video LCD Display

该示例基于 [esp_video](https://github.com/espressif/esp-video-components/tree/master/esp_video) 组件，演示如何把摄像头图像显示到 LCD 屏幕上。应用直接初始化 JD9365 显示屏和 OV5647 摄像头，打开 MIPI-CSI video device，使用 PPA 复制每一帧（帧比屏幕大时居中裁剪），再通过 LVGL adapter dummy-draw 路径送到 LCD frame buffer。IPA 配置 `main/ov5647_custom.json` 增加了灰度世界自动白平衡。

## ESP-IDF 要求

- 该示例要求 ESP-IDF 5.5 或更高版本。
- 工程依赖 `esp_video`、`esp_lvgl_adapter` 和 `waveshare/esp_lcd_jd9365_10_1`。
- 请按照 [ESP-IDF 编程指南](https://docs.espressif.com/projects/esp-idf/zh_CN/latest/esp32/get-started/index.html) 设置开发环境。**我们强烈建议**先 [构建第一个工程](https://docs.espressif.com/projects/esp-idf/zh_CN/latest/esp32/get-started/index.html#build-your-first-project)，熟悉 ESP-IDF 并确保环境正确。

### 前置条件

* ESP32-P4-WIFI6-DB-Touch-LCD-10.1 和受 `esp_video` 支持的 MIPI-CSI 摄像头。
* 用于供电和烧录的 USB-C 线。

### 配置工程

运行 `idf.py menuconfig`，配置摄像头传感器和视频流水线选项。

MIPI-CSI 摄像头的 SCCB/I2C 总线在 `main/main.c` 的 `app_main()` 中创建（I2C 端口 1），修改引脚请改这里，不在 menuconfig 中：

| 信号 | GPIO |
| --- | --- |
| SCL | GPIO8 |
| SDA | GPIO7 |

在 `Espressif Camera Sensors` 配置菜单中，选择与你的硬件匹配的摄像头传感器。具体选择以当前 `sdkconfig` 为准，例如：

```text
Component config  --->
    Espressif Camera Sensors Configurations  --->
        [*] OV5647  ---->
            Default format select for MIPI  --->
                (X) RAW8 800x1280 50fps, MIPI input
```

如果使用 SC2336 或其他传感器，请把传感器选择和输出格式改成与摄像头模块匹配。

### 构建和烧录

构建工程并烧录到开发板，然后运行监视工具查看串口输出（将 `PORT` 替换为开发板串口名）：

```bash
idf.py set-target esp32p4
idf.py -p PORT flash monitor
```

输入 `Ctrl-]` 退出串口监视器。

完整配置和使用 ESP-IDF 构建工程的步骤，请参见 [ESP-IDF 入门指南](https://docs.espressif.com/projects/esp-idf/zh_CN/latest/get-started/index.html)。

### 预期行为

显示背光点亮，LCD 显示实时摄像头图像。日志会打印 video driver 版本、设备名、总线信息，以及检测到的帧宽高。帧分配在 PSRAM 中，由 PPA 复制到三重 LCD frame buffer。日志每秒打印一次帧率以及 PPA 和 flush 耗时。

### 排障

- 先运行 [mipi_dsi](../mipi_dsi/) 验证 LCD 路径。
- 如果出现 `video cam open failed`，检查摄像头 FPC 方向、传感器供电、SCCB/I2C 引脚和已选传感器型号。
- 确认 PSRAM 已启用且稳定；摄像头 buffer 会从 PSRAM 分配。
- 如果图像裁剪、镜像或旋转不正确，请调整传感器输出格式或 `main/main.c` 中的 PPA 操作。不要打开 OV5647 的 VFLIP，它会改变 Bayer 顺序导致颜色错误。
