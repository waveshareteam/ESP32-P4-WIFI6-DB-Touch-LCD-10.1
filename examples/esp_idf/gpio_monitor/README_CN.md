# GPIO 输入输出监控

[English Version](./README.md)

这个示例使用 LVGL 在屏幕上以自适应网格显示选定 GPIO 的实时电平。

## 使用方式

例程不依赖 BSP，直接通过 `esp_lcd_jd9365_10_1` 驱动 JD9365 屏幕、通过 `esp_lcd_touch_gt911` 驱动 GT911 触摸，并通过 `esp_lvgl_adapter` 注册到 LVGL。扩展排针上的 GPIO 列表定义在 `main/main.c` 的 `s_header_gpios[]` 中。

在 `gpio_monitor_config_t` 中设置初始模式：

```c
.initial_mode = GPIO_MODE_INPUT,
```

顶部的 `lv_switch` 用于切换运行模式：关闭时为 `GPIO_MODE_INPUT`，打开时为 `GPIO_MODE_OUTPUT`。所有监控 GPIO 同时切换。输入模式下卡片显示 `GPIOx IN:0/1`，输出模式下显示 `GPIOx OUT:0/1`；输出模式下点击卡片可以翻转输出电平。高电平使用绿色卡片，低电平使用深色卡片；GPIO 数量较多时，网格支持上下滚动。

## 界面布局

- 列数根据 GPIO 总数和屏幕宽度自动选择。
- 卡片宽度由网格列数和屏幕宽度决定。
- 卡片高度由行数和屏幕高度决定；数量过多时保持最小高度并滚动。
- 高电平使用绿色卡片，低电平使用深色卡片。

## 接口

`components/gpio_monitor/include/gpio_monitor.h` 提供：

- `gpio_monitor_init()`：根据用户传入的 GPIO 配置完成 GPIO 初始化。应在显示启动前调用。
- `gpio_monitor_start_ui()`：在显示启动后创建 LVGL 界面，调用时需持有 `esp_lv_adapter_lock()`。
- `gpio_monitor_set_output_enabled()`：通过代码切换所有 GPIO 的 `GPIO_MODE_INPUT`/`GPIO_MODE_OUTPUT`；界面顶部开关使用的就是这个接口。
- `gpio_monitor_set_level()`：给已经配置为输出的 GPIO 设置电平。

当前示例默认以 `GPIO_MODE_INPUT` 初始化。请根据实际板级连接确认这些 GPIO 没有被 LCD、触摸、摄像头或其他外设占用。

## 构建和烧录

```bash
cd examples/esp_idf/gpio_monitor
idf.py set-target esp32p4
idf.py build
idf.py -p PORT flash monitor
```

将 `PORT` 替换为实际串口号。
