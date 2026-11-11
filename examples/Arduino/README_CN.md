# Arduino 示例

[English](README.md)

使用 Arduino IDE 或 Arduino CLI 编译和烧录 Waveshare ESP32-P4-WIFI6-DB Touch LCD 10.1 的示例。CLI 命令在本目录运行。

## 环境要求

- Arduino IDE 或 Arduino CLI，以及 Arduino-ESP32。[esp32](esp32/README_CN.md) 中的板级文件以 Arduino-ESP32 3.3.11 为基线。
- 根据各示例的 README 安装所需库。
- 使用 WSL 时，烧录前需将开发板的 USB 设备连接到 WSL。

使用 CLI 时，可以检查安装：

```bash
arduino-cli version
arduino-cli core list
```

## 使用 Arduino IDE

1. 安装 Arduino IDE，在开发板管理器中安装 ESP32 开发板支持包。项目板级文件的版本基线是 3.3.11。
2. 在 **文件 → 首选项**中查看 sketchbook 位置。如果开发板菜单中没有 `Waveshare ESP32-P4-WIFI6-DB Touch LCD 10.1`，需先安装本项目的板级定义：
   - 找到 IDE 使用的 Arduino-ESP32 平台目录，以及其中的 `boards.txt` 和 `variants` 目录。通过开发板管理器安装的平台通常位于 Arduino 数据目录的 `packages/esp32/hardware/esp32/<版本>`；手动安装时可位于 sketchbook 的 `hardware/espressif/esp32`。
   - 用文本编辑器打开本项目 `esp32/boards.txt`，将全部内容复制到平台目录的 `boards.local.txt`。如果文件已存在，请保留原有内容并追加；先检查是否已有同名条目。
   - 将本项目 `esp32/variants/waveshare_esp32_p4_wifi6_db_touch_lcd_10_1` 文件夹复制到平台的 `variants` 目录。如果目标已存在，先检查其内容。
   - 重启 Arduino IDE。开发板管理器升级或重装平台后，可能需要重新安装这些文件。
3. 选择 **工具 → 开发板**中的 `Waveshare ESP32-P4-WIFI6-DB Touch LCD 10.1`，再通过 **工具 → 端口**选择开发板串口。
4. 使用 **文件 → 打开**，打开示例目录中的同名 `.ino` 文件，例如 `board_check/board_check.ino`。按该示例的 README 安装依赖并设置所需开发板选项。
5. 点击 **验证**编译，点击 **上传**烧录。打开 **工具 → 串口监视器**查看输出；`board_check` 使用 115200 波特率。

如果 IDE 运行在 WSL 中，需要先将 USB 设备连接到 WSL；Windows 版 IDE 使用 Windows 的开发板包和串口。

## 使用 Arduino CLI

## 板级配置

如果开发板已出现在列表中，可以跳过本节：

```bash
arduino-cli board listall | rg 'Waveshare ESP32-P4-WIFI6-DB Touch LCD 10.1'
```

以下命令假定 Arduino sketchbook 位于 `~/Arduino`，Arduino-ESP32 安装在 `~/Arduino/hardware/espressif/esp32`。请根据实际安装位置检查路径。命令会创建 `boards.local.txt` 并复制 variant。**如果任一目标已存在，先检查内容，不要覆盖已有配置。**

```bash
BOARD="$PWD/esp32"
CORE="$HOME/Arduino/hardware/espressif/esp32"
BOARD_ID=waveshare_esp32_p4_wifi6_db_touch_lcd_10_1

test ! -e "$CORE/boards.local.txt" && test ! -e "$CORE/variants/$BOARD_ID" && \
  cp "$BOARD/boards.txt" "$CORE/boards.local.txt" && \
  cp -r "$BOARD/variants/$BOARD_ID" "$CORE/variants/"
```

再次运行上面的 `board listall` 命令，确认开发板的 FQBN 为 `espressif:esp32:waveshare_esp32_p4_wifi6_db_touch_lcd_10_1`。

## 编译

以编译 `board_check` 为例：

```bash
arduino-cli compile -b espressif:esp32:waveshare_esp32_p4_wifi6_db_touch_lcd_10_1 ./board_check
```

将 `./board_check` 换成其他示例目录即可编译对应示例。可选：添加 `-v` 查看详细编译过程：

```bash
arduino-cli compile -v -b espressif:esp32:waveshare_esp32_p4_wifi6_db_touch_lcd_10_1 ./board_check
```

库依赖和开发板选项请查看各示例的 README。

## 烧录

查看串口：

```bash
arduino-cli board list
```

烧录已编译的示例，并将 `/dev/ttyACM0` 替换为实际端口：

```bash
arduino-cli upload -p /dev/ttyACM0 -b espressif:esp32:waveshare_esp32_p4_wifi6_db_touch_lcd_10_1 ./board_check
```

修改代码后需要重新编译；编译和烧录时应使用相同的开发板选项。

## 串口监视

`board_check` 使用 115200 波特率：

```bash
arduino-cli monitor -p /dev/ttyACM0 -c baudrate=115200
```

如果错过启动输出，可以按开发板的 reset 按钮。按 `Ctrl+C` 退出。
