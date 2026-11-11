# LVGL 示例

[English Version](./README.md)

在 ESP32-P4-WIFI6-DB-Touch-LCD-10.1 的 800 x 1280 JD9365 屏上运行 `lv_demo_benchmark`。`esp_board_manager` 初始化显示屏和背光，`esp_lvgl_adapter` 管理 LVGL 显示和任务，直接运行一次性能测试。

## 硬件要求

- ESP32-P4-WIFI6-DB-Touch-LCD-10.1。

显示屏通过 `esp_board_manager` 初始化，硬件描述来自仓库中的板级定义 [`components/waveshare_boards/esp32_p4_wifi6_db_touch_lcd_10_1`](../../../components/waveshare_boards/esp32_p4_wifi6_db_touch_lcd_10_1/)。应用直接读取该描述并初始化以下设备：

| Board Manager 设备 | 用途 |
| --- | --- |
| `display_lcd` | 2 lane MIPI-DSI 连接的 JD9365 屏，包含 DSI 总线和 MIPI PHY LDO。分辨率、色深和帧缓冲数量（`num_fbs`）都取自这里 |
| `lcd_brightness` | GPIO30 背光 PWM，按 `default_percent` 点亮 |

本示例不使用触摸。

## 构建和烧录

需要 ESP-IDF >= 6.0。

```bash
cd examples/esp_idf/lvgl_demo
idf.py bmgr -b esp32_p4_wifi6_db_touch_lcd_10_1 -c ../../../components/waveshare_boards
idf.py build
idf.py -p COMx flash monitor
```

`bmgr` 命令使用板级定义生成 `components/gen_bmgr_codes`，并选择 `esp32p4` 目标。修改板级 YAML 后需要重新运行。板级默认配置（32 MB Flash、HEX PSRAM）只在 `sdkconfig` 不存在时生效，因此请先删除旧的 `sdkconfig`。将 `COMx` 替换为实际串口号。

## 预期串口输出

```text
I (...) lvgl_benchmark: LVGL benchmark started
```

随后屏幕依次运行 LVGL benchmark 的各个场景，结束后在屏幕上显示测试结果。

## 排障

- 如果编译时找不到 `esp_board_manager_includes.h` 或设备名，先运行上面的 `idf.py bmgr` 命令。
- 如果日志正常但屏幕是黑的，检查背光（`lcd_brightness`）。
