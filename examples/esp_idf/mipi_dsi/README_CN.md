# 显示色条

[English Version](./README.md)

在 ESP32-P4-WIFI6-DB-Touch-LCD-10.1 的 10.1 英寸 800 x 1280 JD9365 MIPI-DSI 屏上显示色条。这是本仓库中最简单的显示 bring-up 示例。

## 难度

中级。

## 硬件要求

- ESP32-P4-WIFI6-DB-Touch-LCD-10.1。

显示屏直接使用 [`waveshare/esp_lcd_jd9365_10_1`](https://components.espressif.com/components/waveshare/esp_lcd_jd9365_10_1) 面板驱动和 ESP-IDF `esp_lcd` 接口初始化，不依赖 BSP 或 Board Manager。应用不会创建 LVGL UI，只初始化面板，并请求 DPI 面板驱动生成硬件垂直色条图案。

| 信号 | 配置 |
| --- | --- |
| MIPI-DSI | 2 lane，每 lane 1500 Mbps，RGB565 |
| MIPI PHY 供电 | 内部 LDO 通道 3，2500 mV |
| LCD 复位 | GPIO31 |
| 背光 | GPIO30 LEDC PWM，5 kHz，10 位分辨率，默认亮度 100% |

引脚和时序定义在 [`main/test_esp_lcd_jd9365.c`](main/test_esp_lcd_jd9365.c) 文件开头。

## 构建和烧录

```bash
cd examples/esp_idf/mipi_dsi
idf.py set-target esp32p4
idf.py build
idf.py -p COMx flash monitor
```

将 `COMx` 替换为实际串口号。

## 预期行为

串口日志应显示：

```text
Initialize LCD device
Display jd9365: 800x1280, 16 bpp
Show color bar pattern drawn by hardware
```

LCD 应点亮背光并显示垂直色条。

## 排障

- 确认已启用 PSRAM，DPI 帧缓冲从 PSRAM 分配。
- 如果日志正常但屏幕是黑的，检查背光。修改 `LCD_BK_LIGHT_DEFAULT_PERCENT` 可调整亮度。
- 检查 `main/test_esp_lcd_jd9365.c` 中的复位引脚、LDO 通道和视频时序。
