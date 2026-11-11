<div align="center">
  <h1>ESP32-P4-WIFI6-DB-Touch-LCD-10.1</h1>
  <p><strong>搭载 ESP32-P4NRW32X 和 ESP32-C5-MINI-1 Wi-Fi 6 协处理器，配备 10.1 英寸 800 × 1280 MIPI-DSI 电容触摸屏的开发板</strong></p>
  <p>
    <a href="README.md">English</a> ·
    <a href="docs/hw-reference/USER_GUIDE_CN.md">📚 文档</a> ·
    <a href="firmware/">📦 固件</a> ·
    <a href="examples/esp_idf/">🧩 ESP-IDF 示例</a> ·
    <a href="examples/Arduino/">🔧 Arduino 示例</a>
  </p>
</div>

---

## ✨ 概述

本仓库提供 ESP-IDF 板级支持包（BSP）、ESP Board Manager 板卡定义、ESP-IDF 与 Arduino 示例，以及 ESP-Brookesia 综合应用源码。

开发板提供 MIPI-DSI 显示、GT911 触摸、MIPI-CSI 摄像头接口、ES8389 音频、PCF85063A 实时时钟、microSD、USB Host、TWAI/CAN、RS232、RS485 和 GPIO 排针。

引脚和接口说明见 [BSP README](components/waveshare_bsp/README.md) 与 [API 参考](components/waveshare_bsp/API.md)。

## 🖥️ 硬件概况

| 项目 | 说明 |
| --- | --- |
| 主控 | ESP32-P4NRW32X |
| 无线 | ESP32-C5-MINI-1 · ESP-Hosted SDIO · Wi-Fi 6 · Bluetooth LE |
| 显示 | 10.1 英寸 · 800 × 1280 · MIPI-DSI · JD9365 |
| 触摸 | GT911 · I2C |
| 摄像头接口 | MIPI-CSI |
| 音频 | ES8389 编解码器、双麦克风、双路扬声器功放和耳机接口 |
| 实时时钟 | PCF85063A · I2C |
| 存储 | microSD · SDMMC · SPIFFS |
| 通信接口 | USB Host · TWAI/CAN (TJA1051T/3/1J) · RS232 (SP3232EEN) · RS485 (SP3485EN / SIT3088EESA) |
| 板级支持 | [BSP](components/waveshare_bsp/) · [Board Manager](components/waveshare_boards/) |
| Flash | 32 MB · GD25Q256EYIGR |
| ESP-IDF target | `esp32p4` |

### ESP-IDF LCD 配置

| 面板 | 控制器 | 分辨率 |
| --- | --- | --- |
| 10.1 英寸 MIPI-DSI 电容触摸屏 | JD9365 / GT911 | 800 × 1280 |

构建前请确认显示配置匹配本开发板的 JD9365 竖屏面板。

## 🚀 快速开始

安装并加载 ESP-IDF 环境后，从仓库根目录运行：

```bash
cd examples/esp_idf/board_check
idf.py set-target esp32p4
idf.py build
idf.py -p PORT flash monitor
```

将 `PORT` 替换为实际串口号，例如 Windows 上的 `COM7` 或 Linux 上的 `/dev/ttyACM0`。

推荐使用 ESP-IDF >= 5.5 作为基线（[lvgl_demo](examples/esp_idf/lvgl_demo/) 要求 >= 6.0）。各工程的具体版本约束、依赖和配置以其 README、`idf_component.yml` 和 `sdkconfig.defaults` 为准。

## 🧪 ESP-IDF 示例

| 示例 | 功能 |
| --- | --- |
| [board_check](examples/esp_idf/board_check/) | 开发板信息检查 |
| [gpio_monitor](examples/esp_idf/gpio_monitor/) | GPIO 输入/输出监视（LVGL 界面） |
| [i2c_tools](examples/esp_idf/i2c_tools/) | I2C 扫描和控制台工具 |
| [sdmmc](examples/esp_idf/sdmmc/) | microSD 卡读写 |
| [uart_rs232](examples/esp_idf/uart_rs232/) | RS232 回环收发 |
| [uart_rs485](examples/esp_idf/uart_rs485/) | RS485 半双工回环收发 |
| [twai_network](examples/esp_idf/twai_network/) | TWAI/CAN 发送节点和只听节点 |
| [mipi_dsi](examples/esp_idf/mipi_dsi/) | MIPI-DSI LCD 测试色条 |
| [mipi_csi](examples/esp_idf/mipi_csi/) | MIPI-CSI 摄像头采集和显示 |
| [lvgl_demo](examples/esp_idf/lvgl_demo/) | JD9365 屏幕上的 LVGL 性能测试 |
| [rtc](examples/esp_idf/rtc/) | PCF85063A 实时时钟 |
| [i2s_codec](examples/esp_idf/i2s_codec/) | ES8389 音频播放和录音 |
| [usb_extend_screen](examples/esp_idf/usb_extend_screen/) | Windows USB 扩展屏 |
| [wifi_sta](examples/esp_idf/wifi_sta/) | ESP32-C5 ESP-Hosted Wi-Fi STA |
| [bleprph_gatt](examples/esp_idf/bleprph_gatt/) | ESP-Hosted BLE 外设和 GATT 服务 |

## 🧪 Arduino 示例

本地板卡文件和参数见 [Arduino 板卡说明](examples/Arduino/esp32/README.md)。选择 `Waveshare ESP32-P4-WIFI6-DB Touch LCD 10.1`，并按各示例 README 配置依赖。

| 示例 | 功能 |
| --- | --- |
| [board_check](examples/Arduino/board_check/) | 芯片、Flash、PSRAM 和内存信息 |
| [gpio](examples/Arduino/gpio/) | GPIO 控制 |
| [i2c](examples/Arduino/i2c/) | I2C 设备扫描 |
| [i2s](examples/Arduino/i2s/) | ES8389 音频回环 |
| [mipi_csi](examples/Arduino/mipi_csi/) | 摄像头采集和显示 |
| [mipi_dsi](examples/Arduino/mipi_dsi/) | LCD 初始化和测试色条 |
| [rtc](examples/Arduino/rtc/) | PCF85063A 时间读写 |
| [sdmmc](examples/Arduino/sdmmc/) | microSD 文件读写 |
| [twai](examples/Arduino/twai/) | TWAI/CAN 接收 |
| [uart/rs232](examples/Arduino/uart/rs232/) | USB 串口与 RS232 桥接 |
| [uart/rs485](examples/Arduino/uart/rs485/) | USB 串口与 RS485 桥接 |

## 📡 ESP32-P4 与 ESP32-C5 无线通信

ESP32-C5-MINI-1 通过四位 SDIO 与 ESP32-P4NRW32X 连接，提供 Wi-Fi 6 和 Bluetooth LE。SDIO CLK=GPIO18、CMD=GPIO19、D0–D3=GPIO14–17，EN=GPIO54，唤醒信号=GPIO6。无线功能通过 ESP-Hosted 提供，需要 P4 Host 应用与兼容的 C5 协处理器固件配合运行。烧录 P4 应用不会更新 C5 固件。使用说明见 [Wi-Fi 示例](examples/esp_idf/wifi_sta/) 和 [BLE 示例](examples/esp_idf/bleprph_gatt/)，两者都包含 P4 端的 `mcu_host` 工程和 C5 端的 `cp` 工程。

## 📦 固件

[`firmware/brookesia/`](firmware/brookesia/) 提供 ESP-Brookesia Phone UI 综合应用，集成绘图、频谱、音乐、摄像头、设置、UART、CAN 和 GPIO 工具。使用说明见 [Brookesia README](firmware/brookesia/README_CN.md)。

## 🗂️ 仓库结构

| 路径 | 用途 |
| --- | --- |
| [`examples/esp_idf/`](examples/esp_idf/) | ESP-IDF 独立示例工程 |
| [`examples/Arduino/`](examples/Arduino/) | Arduino 示例和本地板级文件 |
| [`firmware/brookesia/`](firmware/brookesia/) | ESP-Brookesia 综合应用 |
| [`components/waveshare_bsp/`](components/waveshare_bsp/) | 本地板级支持包 |
| [`components/waveshare_boards/`](components/waveshare_boards/) | Board Manager 板级配置 |
| [`docs/`](docs/) | 中英文硬件参考文档 |
| [`asserts/`](asserts/) | 文档图片资源 |

## 📚 文档

- [硬件参考](docs/hw-reference/USER_GUIDE_CN.md)：板载资源、外设配置、GPIO 分配和使用说明。
- [BSP README](components/waveshare_bsp/README.md) · [API](components/waveshare_bsp/API.md).
- [Board Manager](components/waveshare_boards/).
- [Arduino 板卡文件说明](examples/Arduino/esp32/README.md).
- [ESP-Brookesia](firmware/brookesia/README_CN.md).

## 🤝 支持与贡献

欢迎反馈问题或提交改进建议。请附上板卡版本、连接的屏幕或摄像头、ESP-IDF 或 Arduino-ESP32 版本、示例路径、复现步骤、预期与实际现象，以及相关日志。提交 Issue 或 PR 时可使用仓库模板。
