# Repository instructions

## Development entry points

- `examples/esp_idf/` contains separate ESP-IDF projects; `examples/Arduino/` contains Arduino sketches and local board definitions. `firmware/brookesia/` is the integrated application.
- The board's ESP-IDF target is `esp32p4`; wireless examples `wifi_sta` and `bleprph_gatt` also contain separate `mcu_host` and `cp` projects. Check the selected project's target and README before building or flashing.
- For a minimal ESP-IDF starting point, see `examples/esp_idf/board_check/README.md`. From that project directory, its documented build commands are `idf.py set-target esp32p4` and `idf.py build`; obtain explicit user confirmation before executing any `idf.py` command.
- For Arduino setup, board selection, compile and upload commands, see `examples/Arduino/README.md`. Check each sketch's own dependencies and board options.
- Check the selected project's `idf_component.yml`, `sdkconfig.defaults*`, and, when present, `dependencies.lock` for its actual requirements. For example, `examples/esp_idf/lvgl_demo/main/idf_component.yml` has a different IDF minimum from the BSP manifest; do not assume a repository-wide version.
- Board Manager examples such as `lvgl_demo` and `i2s_codec` generate `components/gen_bmgr_codes` from `components/waveshare_boards/esp32_p4_wifi6_db_touch_lcd_10_1/`. Follow the example README to regenerate after changing board YAML. Existing `sdkconfig` can prevent board defaults from being applied; inspect it before changing or removing it.
- Hardware connections and pins are documented in `docs/hw-reference/`; BSP interfaces and configuration live in `components/waveshare_bsp/`. Verify the relevant source before changing hardware settings.
- `build/`, `managed_components/`, `sdkconfig`, `dependencies.lock`, and generated Board Manager code are ignored by `.gitignore`; inspect the actual Git status and tracked files rather than assuming every generated or lock file is absent from version control.

## Working conventions

- Check the affected example or firmware README, dependency manifests, and configuration before changing it; projects have independent build procedures.
- Treat this checkout as the authority for board details and dependency versions. Distinguish static checks, builds, and tests on the actual board in validation reports.
- Keep paired English and Chinese documentation consistent when changing documented behavior.

## Maintenance

When adding or changing an ESP-IDF example:

- Register new source files and direct component requirements in the relevant `CMakeLists.txt`; declare external managed dependencies and IDF version constraints in that example's `idf_component.yml` where applicable. These serve different purposes: for example, `lvgl_demo/main/CMakeLists.txt` declares `PRIV_REQUIRES`, while its `idf_component.yml` declares managed dependencies.
- Check `sdkconfig.defaults*`, `Kconfig.projbuild`, and `partitions.csv` where present when changing target, features, selectable modes, or storage. For example, `i2s_codec/main/CMakeLists.txt` selects source files using `CONFIG_EXAMPLE_MODE_ECHO`; its documented partition setup must match its configuration.
- Update the example's `README.md` and `README_CN.md` together when requirements, configuration, commands, expected behavior, or wiring change. If an example is added, renamed, removed, or its stated minimum IDF version changes, also check the example lists and version statement in root `README.md` and `README_CN.md`.
- For Board Manager examples, keep the generation command and board path in both READMEs aligned with the manifest and board definition. Never assume generated files or an existing `sdkconfig` reflect changed defaults.

When a board pin or peripheral configuration changes, inspect each representation of the affected signal and update only those that apply to the actual hardware revision:

- BSP pin macros: `components/waveshare_bsp/include/bsp/esp32_p4_wifi6_db_touch_lcd_10_1.h`; inspect their use in the BSP implementation as well.
- Board Manager: bus pins and peripheral settings in `components/waveshare_boards/esp32_p4_wifi6_db_touch_lcd_10_1/board_peripherals.yaml`; device-specific pins, device bindings and configuration in `board_devices.yaml`. The shared I2C, I2S, LCD backlight, LCD reset and touch signals appear in these files.
- Arduino board constants: `examples/Arduino/esp32/variants/waveshare_esp32_p4_wifi6_db_touch_lcd_10_1/pins_arduino.h`. Search affected sketches for both these constants and hard-coded pins; for example, `i2c/i2c.ino` uses `SDA`/`SCL`, and `mipi_dsi/mipi_dsi.ino` uses `LCD_RST`/`LCD_BL`.
- Hardware descriptions: check both `docs/hw-reference/USER_GUIDE.md` and `docs/hw-reference/USER_GUIDE_CN.md`, plus relevant example READMEs. Document actual physical wiring, not merely a software configuration change.
- After changing Board Manager YAML, follow the affected example's README for regenerating `gen_bmgr_codes`. Check consumers and tests for affected signals; report whether the result was verified on the board.

| Change | Review related files |
| --- | --- |
| Board Manager definitions in `components/waveshare_boards/esp32_p4_wifi6_db_touch_lcd_10_1/` | Board Manager examples such as `examples/esp_idf/i2s_codec/` and `examples/esp_idf/lvgl_demo/`; relevant English and Chinese hardware guides under `docs/` |
| BSP public API or dependencies in `components/waveshare_bsp/` | Its `include/`, `API.md`, `CMakeLists.txt`, `idf_component.yml`, and affected consumers |
| Example behavior or setup | The example's configuration and README files in both languages, where present |

## Done means

- Run `git diff --check` and require exit code 0 for tracked changes. For each new untracked text file, run `git diff --no-index --check /dev/null <file>` and inspect its output for whitespace errors; exit code 1 alone indicates files differ.
- For `examples/esp_idf/board_check/` changes, request explicit confirmation before running `idf.py build` in that directory after configuring its target as documented. A successful build exits 0; otherwise record that it was not run or failed.
- Record the exact project-specific validation commands and results; distinguish static checks, build results and board tests. An unrun check is not a pass.

## Never merges without a human

A human must read and approve the diff before it is merged when it changes:

- `components/waveshare_boards/esp32_p4_wifi6_db_touch_lcd_10_1/` board hardware configuration.
- `components/waveshare_bsp/include/` public headers or its `idf_component.yml` dependency manifest.
