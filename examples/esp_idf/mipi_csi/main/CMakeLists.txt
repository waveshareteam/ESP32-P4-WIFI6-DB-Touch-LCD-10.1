idf_component_register(SRCS "main.c" "app_video.c"
                       PRIV_INCLUDE_DIRS "."
                       PRIV_REQUIRES esp_lcd esp_driver_i2c esp_driver_ledc esp_driver_ppa esp_mm esp_timer esp_hw_support)
