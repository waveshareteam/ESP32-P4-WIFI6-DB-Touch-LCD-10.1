<div align="center">
  <h1>ESP32-P4-WIFI6-DB-Touch-LCD-10.1</h1>
  <p><strong>ESP32-P4NRW32X development board with an ESP32-C5-MINI-1 Wi-Fi 6 coprocessor and a 10.1-inch 800 × 1280 MIPI-DSI capacitive touch display</strong></p>
  <p>
    <a href="README_CN.md">简体中文</a> ·
    <a href="docs/hw-reference/USER_GUIDE.md">📚 Documentation</a> ·
    <a href="firmware/">📦 Firmware</a> ·
    <a href="examples/esp_idf/">🧩 ESP-IDF Examples</a> ·
    <a href="examples/Arduino/">🔧 Arduino Examples</a>
  </p>
</div>

---

## ✨ Overview

This repository provides an ESP-IDF board support package (BSP), an ESP Board Manager board definition, ESP-IDF and Arduino examples, and an integrated ESP-Brookesia application.

The board provides a MIPI-DSI display, GT911 touch, a MIPI-CSI camera interface, ES8389 audio, a PCF85063A real-time clock, microSD, USB Host, TWAI/CAN, RS232, RS485, and a GPIO expansion header.

For pin assignments and interfaces, see the [BSP README](components/waveshare_bsp/README.md) and [API reference](components/waveshare_bsp/API.md).

## 🖥️ Hardware Overview

| Item | Details |
| --- | --- |
| MCU | ESP32-P4NRW32X |
| Wireless | ESP32-C5-MINI-1 · ESP-Hosted SDIO · Wi-Fi 6 · Bluetooth LE |
| Display | 10.1-inch · 800 × 1280 · MIPI-DSI · JD9365 |
| Touch | GT911 · I2C |
| Camera interface | MIPI-CSI |
| Audio | ES8389 codec, two microphones, dual speaker amplifiers, and headset jack |
| Real-time clock | PCF85063A · I2C |
| Storage | microSD · SDMMC · SPIFFS |
| Communication | USB Host · TWAI/CAN (TJA1051T/3/1J) · RS232 (SP3232EEN) · RS485 (SP3485EN / SIT3088EESA) |
| Board support | [BSP](components/waveshare_bsp/) · [Board Manager](components/waveshare_boards/) |
| Flash | 32 MB · GD25Q256EYIGR |
| ESP-IDF target | `esp32p4` |

### ESP-IDF LCD Configuration

| Panel | Controller | Resolution |
| --- | --- | --- |
| 10.1-inch MIPI-DSI capacitive touch display | JD9365 / GT911 | 800 × 1280 |

Before building, make sure the display configuration matches this board's JD9365 portrait panel.

## 🚀 Getting Started

Install and activate ESP-IDF, then run from the repository root:

```bash
cd examples/esp_idf/board_check
idf.py set-target esp32p4
idf.py build
idf.py -p PORT flash monitor
```

Replace `PORT` with your serial port, such as `COM7` on Windows or `/dev/ttyACM0` on Linux.

The recommended baseline is ESP-IDF >= 5.5 ([lvgl_demo](examples/esp_idf/lvgl_demo/) requires >= 6.0). Check each project's README, `idf_component.yml`, and `sdkconfig.defaults` for its specific version constraints, dependencies, and configuration.

## 🧪 ESP-IDF Examples

| Example | Focus |
| --- | --- |
| [board_check](examples/esp_idf/board_check/) | Board information |
| [gpio_monitor](examples/esp_idf/gpio_monitor/) | GPIO input/output monitor with an LVGL UI |
| [i2c_tools](examples/esp_idf/i2c_tools/) | I2C scanning and console tools |
| [sdmmc](examples/esp_idf/sdmmc/) | microSD card access |
| [uart_rs232](examples/esp_idf/uart_rs232/) | RS232 echo |
| [uart_rs485](examples/esp_idf/uart_rs485/) | RS485 half-duplex echo |
| [twai_network](examples/esp_idf/twai_network/) | TWAI/CAN sender and listen-only nodes |
| [mipi_dsi](examples/esp_idf/mipi_dsi/) | MIPI-DSI LCD color bars |
| [mipi_csi](examples/esp_idf/mipi_csi/) | MIPI-CSI camera capture and display |
| [lvgl_demo](examples/esp_idf/lvgl_demo/) | LVGL benchmark on the JD9365 display |
| [rtc](examples/esp_idf/rtc/) | PCF85063A real-time clock |
| [i2s_codec](examples/esp_idf/i2s_codec/) | ES8389 audio playback and recording |
| [usb_extend_screen](examples/esp_idf/usb_extend_screen/) | USB extended display for Windows |
| [wifi_sta](examples/esp_idf/wifi_sta/) | ESP32-C5 ESP-Hosted Wi-Fi station |
| [bleprph_gatt](examples/esp_idf/bleprph_gatt/) | ESP-Hosted BLE peripheral with a GATT service |

## 🧪 Arduino Examples

See the [local Arduino board guide (Chinese)](examples/Arduino/esp32/README.md). Select `Waveshare ESP32-P4-WIFI6-DB Touch LCD 10.1` and follow each example README for dependencies.

| Example | Focus |
| --- | --- |
| [board_check](examples/Arduino/board_check/) | Chip, Flash, PSRAM, and memory information |
| [gpio](examples/Arduino/gpio/) | GPIO control |
| [i2c](examples/Arduino/i2c/) | I2C device scanning |
| [i2s](examples/Arduino/i2s/) | ES8389 audio loopback |
| [mipi_csi](examples/Arduino/mipi_csi/) | Camera capture and display |
| [mipi_dsi](examples/Arduino/mipi_dsi/) | LCD initialization and color bars |
| [rtc](examples/Arduino/rtc/) | PCF85063A time access |
| [sdmmc](examples/Arduino/sdmmc/) | microSD file access |
| [twai](examples/Arduino/twai/) | TWAI/CAN reception |
| [uart/rs232](examples/Arduino/uart/rs232/) | USB serial to RS232 bridge |
| [uart/rs485](examples/Arduino/uart/rs485/) | USB serial to RS485 bridge |

## 📡 ESP32-P4 and ESP32-C5 Wireless

ESP32-C5-MINI-1 connects to ESP32-P4NRW32X over four-bit SDIO and provides Wi-Fi 6 and Bluetooth LE. SDIO CLK=GPIO18, CMD=GPIO19, D0–D3=GPIO14–17, EN=GPIO54, and wake-up=GPIO6. Wireless operation uses ESP-Hosted and requires a P4 host application and compatible C5 coprocessor firmware. Flashing the P4 application does not update the C5 firmware. See the [Wi-Fi example](examples/esp_idf/wifi_sta/) and [BLE example](examples/esp_idf/bleprph_gatt/). Each has a `mcu_host` project for the P4 and a `cp` project for the C5.

## 📦 Firmware

[`firmware/brookesia/`](firmware/brookesia/) provides an ESP-Brookesia Phone UI application with drawing, spectrum, music, camera, settings, UART, CAN, and GPIO tools. See the [Brookesia README](firmware/brookesia/README.md).

## 🗂️ Repository Layout

| Path | Purpose |
| --- | --- |
| [`examples/esp_idf/`](examples/esp_idf/) | Standalone ESP-IDF examples |
| [`examples/Arduino/`](examples/Arduino/) | Arduino sketches and local board definitions |
| [`firmware/brookesia/`](firmware/brookesia/) | Integrated ESP-Brookesia application |
| [`components/waveshare_bsp/`](components/waveshare_bsp/) | Local board support package |
| [`components/waveshare_boards/`](components/waveshare_boards/) | Board Manager board configuration |
| [`docs/`](docs/) | Hardware reference in English and Chinese |
| [`asserts/`](asserts/) | Documentation image assets |

## 📚 Documentation

- [Hardware reference](docs/hw-reference/USER_GUIDE.md): onboard resources, peripheral configuration, GPIO allocation, and usage notes.
- [BSP README](components/waveshare_bsp/README.md) · [API](components/waveshare_bsp/API.md).
- [Board Manager](components/waveshare_boards/).
- [Arduino board guide (Chinese)](examples/Arduino/esp32/README.md).
- [ESP-Brookesia](firmware/brookesia/README.md).

## 🤝 Support and Contributions

Questions, improvement suggestions, and reproducible issue reports are welcome. Include the board revision, connected display or camera, ESP-IDF or Arduino-ESP32 version, example path, reproduction steps, expected and actual behavior, and relevant logs. Use the repository Issue and PR templates when submitting.
