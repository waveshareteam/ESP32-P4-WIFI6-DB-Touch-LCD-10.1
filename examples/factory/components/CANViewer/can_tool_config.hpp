#pragma once

#include <cstdint>
#include "driver/gpio.h"

namespace esp_brookesia::apps {

enum class CanOperatingMode : int32_t {
    Normal = 0,
    ListenOnly,
};

enum class CanFrameFilter : int32_t {
    All = 0,
    Standard,
    Extended,
};

// Fixed CAN transceiver routing on the ESP32-P4-WIFI6-DB-Touch-LCD-10.1 board.
constexpr gpio_num_t CAN_TOOL_TX_GPIO = GPIO_NUM_22;
constexpr gpio_num_t CAN_TOOL_RX_GPIO = GPIO_NUM_21;

constexpr bool can_tool_pins_ready()
{
    return CAN_TOOL_TX_GPIO != GPIO_NUM_NC &&
           CAN_TOOL_RX_GPIO != GPIO_NUM_NC;
}

struct CanToolConfig {
    CanOperatingMode mode = CanOperatingMode::Normal;
    int bitrate = 500000;
    CanFrameFilter frame_filter = CanFrameFilter::All;
};

CanToolConfig can_tool_config_load();
bool can_tool_config_save(const CanToolConfig &config);

} // namespace esp_brookesia::apps
