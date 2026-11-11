idf_component_register(
    SRCS main.c
    INCLUDE_DIRS .
    PRIV_REQUIRES gpio_monitor esp_lcd esp_driver_i2c esp_driver_ledc esp_hw_support)
