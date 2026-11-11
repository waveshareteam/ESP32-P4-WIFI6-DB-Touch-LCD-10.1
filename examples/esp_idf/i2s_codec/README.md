# I2S Codec Audio Example

[中文版本](./README_CN.md)

This example uses the ES8389 codec on the Waveshare ESP32-P4-WIFI6-DB-Touch-LCD-10.1 to demonstrate embedded PCM looping and live microphone loopback. Audio is handled with `esp_board_manager` and `esp_codec_dev` only; no player service or decoders are used.

## Components

| Component | Purpose |
| --- | --- |
| `esp_board_manager` | Initialize the audio ADC, DAC, I2S, I2C, and amplifier from the board configuration |
| `esp_codec_dev` | Write PCM to the DAC, read microphone PCM from the ADC, and set volume and MIC gain |

## Build and Flash

From the repository root:

```powershell
cd examples\esp_idf\i2s_codec
idf.py set-target esp32p4
idf.py bmgr -b esp32_p4_wifi6_db_touch_lcd_10_1 -c ../../../components/waveshare_boards
idf.py menuconfig
idf.py build
idf.py -p COMx flash monitor
```

The `bmgr` command generates `components/gen_bmgr_codes` from the board definitions in this repository. Replace `COMx` with the serial port. The project configures 32 MiB of Flash and a 4 MiB `factory` partition in `partitions.csv`; when reusing an existing `sdkconfig`, confirm that this custom partition table is selected.

## Modes and Configuration

Select a mode under `menuconfig → Example Configuration → Example mode`:

| Mode | Usage |
| --- | --- |
| **Embedded PCM loop** (default) | Loop `main/canon.pcm`, embedded in the firmware; no SD card required |
| **echo** | Speak into the microphone to hear live playback through the speaker; AEC is not enabled |

**Voice volume** sets playback volume (0–100, default 60). In echo mode, **Set MIC gain** sets microphone analog gain (0–42 dB in 6 dB steps, default 24 dB).

## Runtime Flow

Board Manager initializes the devices needed by the selected mode. Each mode opens the codec(s) with `esp_codec_dev_open()` and starts a task that reads/writes PCM directly:

| Mode | Audio path |
| --- | --- |
| Embedded PCM | Embedded PCM → `esp_codec_dev_write()` → ES8389 DAC → speaker |
| Echo | Microphone → ES8389 ADC → `esp_codec_dev_read()` → `esp_codec_dev_write()` → ES8389 DAC → speaker |

Both modes use 16 kHz, stereo, 16-bit PCM.
