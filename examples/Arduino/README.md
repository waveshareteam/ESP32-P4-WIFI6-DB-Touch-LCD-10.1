# Arduino Examples

[中文](README_CN.md)

Compile and upload the examples for the Waveshare ESP32-P4-WIFI6-DB Touch LCD 10.1 with Arduino IDE or Arduino CLI. Run CLI commands from this directory.

## Requirements

- Arduino IDE or Arduino CLI, and Arduino-ESP32. The board files in [esp32](esp32/README.md) use Arduino-ESP32 3.3.11 as their baseline.
- Install any libraries required by the individual example; see its README.
- For WSL, attach the board's USB device to WSL before uploading.

For CLI use, check the installation:

```bash
arduino-cli version
arduino-cli core list
```

## Using Arduino IDE

1. Install Arduino IDE and install ESP32 board support through Boards Manager. The project board files use version 3.3.11 as their baseline.
2. Check the sketchbook location under **File → Preferences**. If `Waveshare ESP32-P4-WIFI6-DB Touch LCD 10.1` is missing from the board menu, install the project's board definition:
   - Locate the Arduino-ESP32 platform directory used by the IDE, containing `boards.txt` and `variants`. A Boards Manager installation is normally under `packages/esp32/hardware/esp32/<version>` in the Arduino data directory; a manual installation can be under the sketchbook's `hardware/espressif/esp32`.
   - Open the project's `esp32/boards.txt` in a text editor and copy its contents to `boards.local.txt` in the platform directory. If the file already exists, preserve its contents and append the lines; first check for an existing entry for this board.
   - Copy the project's `esp32/variants/waveshare_esp32_p4_wifi6_db_touch_lcd_10_1` folder into the platform's `variants` directory. Inspect the destination first if it already exists.
   - Restart Arduino IDE. You may need to reinstall these files after upgrading or reinstalling the platform through Boards Manager.
3. Select `Waveshare ESP32-P4-WIFI6-DB Touch LCD 10.1` under **Tools → Board**, then select its serial port under **Tools → Port**.
4. Use **File → Open** to open the matching `.ino` file in an example directory, such as `board_check/board_check.ino`. Install dependencies and set board options as described in that example's README.
5. Click **Verify** to compile and **Upload** to flash. Open **Tools → Serial Monitor** for output; `board_check` uses 115200 baud.

If the IDE runs in WSL, attach the USB device to WSL first. The Windows IDE uses its Windows board packages and serial ports.

## Using Arduino CLI

## Board configuration

If the board is already listed, skip this section:

```bash
arduino-cli board listall | rg 'Waveshare ESP32-P4-WIFI6-DB Touch LCD 10.1'
```

The commands below assume the Arduino sketchbook is `~/Arduino` and Arduino-ESP32 is installed at `~/Arduino/hardware/espressif/esp32`. Check the paths for your installation. They create `boards.local.txt` and copy the variant. **If either destination already exists, inspect it first; do not overwrite existing customizations.**

```bash
BOARD="$PWD/esp32"
CORE="$HOME/Arduino/hardware/espressif/esp32"
BOARD_ID=waveshare_esp32_p4_wifi6_db_touch_lcd_10_1

test ! -e "$CORE/boards.local.txt" && test ! -e "$CORE/variants/$BOARD_ID" && \
  cp "$BOARD/boards.txt" "$CORE/boards.local.txt" && \
  cp -r "$BOARD/variants/$BOARD_ID" "$CORE/variants/"
```

Check that the board appears with the FQBN `espressif:esp32:waveshare_esp32_p4_wifi6_db_touch_lcd_10_1` using the `board listall` command above.

## Compile

For example, compile `board_check`:

```bash
arduino-cli compile -b espressif:esp32:waveshare_esp32_p4_wifi6_db_touch_lcd_10_1 ./board_check
```

Replace `./board_check` with another example directory. Optionally add `-v` to see detailed compilation output:

```bash
arduino-cli compile -v -b espressif:esp32:waveshare_esp32_p4_wifi6_db_touch_lcd_10_1 ./board_check
```

Check each example's README for library dependencies and board options.

## Upload

Find the serial port:

```bash
arduino-cli board list
```

Upload the compiled example, replacing `/dev/ttyACM0` with the actual port:

```bash
arduino-cli upload -p /dev/ttyACM0 -b espressif:esp32:waveshare_esp32_p4_wifi6_db_touch_lcd_10_1 ./board_check
```

Recompile after changing the code. Use the same board options for compilation and upload.

## Serial monitor

`board_check` uses 115200 baud:

```bash
arduino-cli monitor -p /dev/ttyACM0 -c baudrate=115200
```

Press the board's reset button if its startup output was missed. Press `Ctrl+C` to exit.
