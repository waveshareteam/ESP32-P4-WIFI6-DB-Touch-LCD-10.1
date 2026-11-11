/*
 * SPDX-FileCopyrightText: 2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */
#include <string.h>
#include <sys/ioctl.h>

#include "esp_err.h"
#include "esp_log.h"
#include "esp_check.h"
#include "esp_cache.h"
#include "esp_heap_caps.h"
#include "esp_private/esp_cache_private.h"
#include "esp_timer.h"
#include "driver/ppa.h"
#include "app_video.h"

#include "driver/i2c_master.h"
#include "driver/ledc.h"
#include "esp_ldo_regulator.h"
#include "esp_lcd_mipi_dsi.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_jd9365_10_1.h"
#include "esp_lv_adapter.h"
#include "esp_video_init.h"
#include "esp_video_device.h"

#define LCD_WIDTH 800
#define LCD_HEIGHT 1280
#define LCD_ROTATION ESP_LV_ADAPTER_ROTATE_0

// Sensor auto exposure target, 2~235, driver default 0x50, lower is darker.
// White balance is handled automatically by the IPA AWB in main/ov5647_custom.json.
#define CAMERA_AE_TARGET        0x40

#define CAMERA_BUF_NUM          3
#define FPS_REPORT_INTERVAL_US  (1000 * 1000)
static lv_display_t *s_display;
static i2c_master_bus_handle_t s_i2c_bus;
static esp_ldo_channel_handle_t s_dsi_ldo;


#define ALIGN_UP(num, align) (((num) + ((align) - 1)) & ~((align) - 1))

static void camera_video_frame_operation(
    uint8_t *camera_buf,
    uint8_t camera_buf_index,
    uint32_t camera_buf_hes,
    uint32_t camera_buf_ves,
    size_t camera_buf_len,
    void *user_data);

static const char *TAG = "app_main";

static ppa_client_handle_t ppa_srm_handle = NULL;
static size_t data_cache_line_size = 0;

static bool dummy_draw_enabled = true;
static bool dummy_mode_delay_flag = false;

static void camera_image_tune(int cam_fd)
{
    struct v4l2_ext_controls controls = {0};
    struct v4l2_ext_control control[1] = {0};

    controls.ctrl_class = V4L2_CID_CAMERA_CLASS;
    controls.count      = 1;
    controls.controls   = control;
    control[0].id       = V4L2_CID_EXPOSURE;
    control[0].value    = CAMERA_AE_TARGET;
    if (ioctl(cam_fd, VIDIOC_S_EXT_CTRLS, &controls) != 0) {
        ESP_LOGW(TAG, "failed to set AE target");
    }
}

static void calc_ppa_input_offset(uint32_t src_w, uint32_t src_h,
                                  uint32_t dst_w, uint32_t dst_h,
                                  uint32_t *offset_x, uint32_t *offset_y)
{
    *offset_x = (src_w > dst_w) ? (src_w - dst_w) / 2 : 0;
    *offset_y = (src_h > dst_h) ? (src_h - dst_h) / 2 : 0;

    if ((*offset_x + dst_w) > src_w)
    {
        *offset_x = 0;
    }
    if ((*offset_y + dst_h) > src_h)
    {
        *offset_y = 0;
    }
}

void app_main(void)
{
    const i2c_master_bus_config_t i2c_config = {
        .i2c_port = 1, .sda_io_num = 7, .scl_io_num = 8,
        .clk_source = I2C_CLK_SRC_DEFAULT,
    };
    ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_config, &s_i2c_bus));

    const ledc_timer_config_t timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE, .timer_num = LEDC_TIMER_1,
        .duty_resolution = LEDC_TIMER_10_BIT, .freq_hz = 5000, .clk_cfg = LEDC_AUTO_CLK,
    };
    ESP_ERROR_CHECK(ledc_timer_config(&timer));
    const ledc_channel_config_t backlight = {
        .gpio_num = 30, .speed_mode = LEDC_LOW_SPEED_MODE, .channel = LEDC_CHANNEL_1,
        .timer_sel = LEDC_TIMER_1, .duty = 1023,
    };
    ESP_ERROR_CHECK(ledc_channel_config(&backlight));

    const esp_ldo_channel_config_t ldo_config = { .chan_id = 3, .voltage_mv = 2500 };
    ESP_ERROR_CHECK(esp_ldo_acquire_channel(&ldo_config, &s_dsi_ldo));
    const esp_lcd_dsi_bus_config_t bus_config = JD9365_PANEL_BUS_DSI_2CH_CONFIG();
    esp_lcd_dsi_bus_handle_t dsi_bus = NULL;
    ESP_ERROR_CHECK(esp_lcd_new_dsi_bus(&bus_config, &dsi_bus));
    const esp_lcd_dbi_io_config_t io_config = JD9365_PANEL_IO_DBI_CONFIG();
    esp_lcd_panel_io_handle_t io = NULL;
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_dbi(dsi_bus, &io_config, &io));
    esp_lcd_dpi_panel_config_t dpi_config = JD9365_800_1280_PANEL_60HZ_DPI_CONFIG(LCD_COLOR_PIXEL_FORMAT_RGB565);
    dpi_config.num_fbs = esp_lv_adapter_get_required_frame_buffer_count(
                             ESP_LV_ADAPTER_TEAR_AVOID_MODE_DEFAULT_MIPI_DSI, LCD_ROTATION);
    jd9365_vendor_config_t vendor_config = {
        .mipi_config = { .dsi_bus = dsi_bus, .dpi_config = &dpi_config, .lane_num = 2 },
    };
    const esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = 31, .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB,
        .bits_per_pixel = 16, .vendor_config = &vendor_config,
    };
    esp_lcd_panel_handle_t panel = NULL;
    ESP_ERROR_CHECK(esp_lcd_new_panel_jd9365(io, &panel_config, &panel));
    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel));
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel, true));

    const esp_lv_adapter_config_t adapter_config = ESP_LV_ADAPTER_DEFAULT_CONFIG();
    ESP_ERROR_CHECK(esp_lv_adapter_init(&adapter_config));
    esp_lv_adapter_display_config_t display_config = ESP_LV_ADAPTER_DISPLAY_MIPI_DEFAULT_CONFIG(
                panel, io, LCD_WIDTH, LCD_HEIGHT, LCD_ROTATION);
    s_display = esp_lv_adapter_register_display(&display_config);
    ESP_ERROR_CHECK(s_display ? ESP_OK : ESP_FAIL);
    ESP_ERROR_CHECK(esp_lv_adapter_start());
    ESP_ERROR_CHECK(esp_lv_adapter_set_dummy_draw(s_display, dummy_draw_enabled));

    ppa_client_config_t ppa_srm_config = {
        .oper_type = PPA_OPERATION_SRM,
    };
    ESP_ERROR_CHECK(ppa_register_client(&ppa_srm_config, &ppa_srm_handle));

    ESP_ERROR_CHECK(esp_cache_get_alignment(MALLOC_CAP_SPIRAM, &data_cache_line_size));

    const esp_video_init_csi_config_t csi_config = {
        .sccb_config = { .init_sccb = false, .i2c_handle = s_i2c_bus, .freq = 400000 },
        .reset_pin = -1, .pwdn_pin = -1,
        .dont_init_ldo = true,
    };
    const esp_video_init_config_t camera_config = { .csi = &csi_config };
    ESP_ERROR_CHECK(esp_video_init(&camera_config));

    int video_cam_fd0 = app_video_open(ESP_VIDEO_MIPI_CSI_DEVICE_NAME, APP_VIDEO_FMT);
    if (video_cam_fd0 < 0)
    {
        ESP_LOGE(TAG, "video cam open failed");
        return;
    }
    camera_image_tune(video_cam_fd0);

    ESP_LOGI(TAG, "Using LVGL adapter managed frame buffers");
    // 3 buffers so the camera can keep capturing while one frame is copied and one waits for display
    void *camera_buf[CAMERA_BUF_NUM];
    for (int i = 0; i < CAMERA_BUF_NUM; i++)
    {
        camera_buf[i] = heap_caps_aligned_calloc(
            data_cache_line_size,
            1,
            app_video_get_buf_size(),
            MALLOC_CAP_SPIRAM);
        ESP_ERROR_CHECK(camera_buf[i] ? ESP_OK : ESP_ERR_NO_MEM);
    }
    ESP_ERROR_CHECK(app_video_set_bufs(video_cam_fd0, CAMERA_BUF_NUM, (void *)camera_buf));

    ESP_ERROR_CHECK(app_video_register_frame_operation_cb(camera_video_frame_operation));

    ESP_ERROR_CHECK(app_video_stream_task_start(video_cam_fd0, 0, NULL));

}

static void camera_video_frame_operation(
    uint8_t *camera_buf,
    uint8_t camera_buf_index,
    uint32_t camera_buf_hes,
    uint32_t camera_buf_ves,
    size_t camera_buf_len,
    void *user_data)
{
    if (!dummy_draw_enabled || dummy_mode_delay_flag)
    {
        return;
    }

    (void)camera_buf_index;
    (void)camera_buf_len;
    (void)user_data;

    const uint32_t display_width = LCD_WIDTH;
    const uint32_t display_height = LCD_HEIGHT;

    if (display_height == 0)
    {
        ESP_LOGE(TAG, "Display height is zero!");
        return;
    }

    uint32_t in_offset_x = 0;
    uint32_t in_offset_y = 0;
    calc_ppa_input_offset(camera_buf_hes, camera_buf_ves,
                          display_width, display_height,
                          &in_offset_x, &in_offset_y);

    uint32_t in_block_w = (camera_buf_hes > display_width) ? display_width : camera_buf_hes;
    uint32_t in_block_h = (camera_buf_ves > display_height) ? display_height : camera_buf_ves;
    const uint32_t draw_w = in_block_w;
    const uint32_t draw_h = in_block_h;
    const size_t bytes_per_pixel = APP_VIDEO_FMT == APP_VIDEO_FMT_RGB565 ? 2 : 3;
    const size_t frame_buffer_size = ALIGN_UP(display_width * display_height * bytes_per_pixel,
                                              data_cache_line_size);

    void *lcd_frame_buffer = esp_lv_adapter_dummy_draw_get_free_buf(s_display);
    if (lcd_frame_buffer == NULL)
    {
        ESP_LOGE(TAG, "No free LCD frame buffer");
        return;
    }

    if (draw_w < display_width || draw_h < display_height)
    {
        memset(lcd_frame_buffer, 0, frame_buffer_size);
        esp_err_t cache_ret = esp_cache_msync(lcd_frame_buffer,
                                              frame_buffer_size,
                                              ESP_CACHE_MSYNC_FLAG_DIR_C2M);
        if (cache_ret != ESP_OK)
        {
            ESP_LOGE(TAG, "LCD frame buffer cache sync failed: %s", esp_err_to_name(cache_ret));
            return;
        }
    }

    ppa_srm_oper_config_t srm_config = {
        .in.buffer = camera_buf,
        .in.pic_w = camera_buf_hes,
        .in.pic_h = camera_buf_ves,
        .in.block_w = in_block_w,
        .in.block_h = in_block_h,
        .in.block_offset_x = in_offset_x,
        .in.block_offset_y = in_offset_y,
        .in.srm_cm = APP_VIDEO_FMT == APP_VIDEO_FMT_RGB565 ? PPA_SRM_COLOR_MODE_RGB565 : PPA_SRM_COLOR_MODE_RGB888,

        .out.buffer = lcd_frame_buffer,
        .out.buffer_size = frame_buffer_size,
        .out.pic_w = display_width,
        .out.pic_h = display_height,
        .out.block_offset_x = 0,
        .out.block_offset_y = 0,
        .out.srm_cm = APP_VIDEO_FMT == APP_VIDEO_FMT_RGB565 ? PPA_SRM_COLOR_MODE_RGB565 : PPA_SRM_COLOR_MODE_RGB888,

        .rotation_angle = PPA_SRM_ROTATION_ANGLE_0,
        .scale_x = 1,
        .scale_y = 1,
        .mirror_x = 0,
        .mirror_y = 0,
        .rgb_swap = 0,
        .byte_swap = 0,
        .mode = PPA_TRANS_MODE_BLOCKING,
    };

    int64_t t_ppa = esp_timer_get_time();
    esp_err_t ret = ppa_do_scale_rotate_mirror(ppa_srm_handle, &srm_config);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "PPA SRM failed: %d", ret);
        return;
    }
    int64_t t_flush = esp_timer_get_time();

    if (dummy_draw_enabled && !dummy_mode_delay_flag)
    {
        ret = esp_lv_adapter_dummy_draw_flush_buf(s_display, lcd_frame_buffer);

        if (ret != ESP_OK)
        {
            ESP_LOGE(TAG, "Dummy draw flush failed: %s", esp_err_to_name(ret));
        }
    }
    int64_t t_end = esp_timer_get_time();

    // Report frame rate and where the time goes once per second
    static int64_t s_report_start = 0;
    static uint32_t s_frames = 0;
    static int64_t s_ppa_us = 0;
    static int64_t s_flush_us = 0;
    if (s_report_start == 0) {
        s_report_start = t_ppa;
    }
    s_frames++;
    s_ppa_us += t_flush - t_ppa;
    s_flush_us += t_end - t_flush;
    if (t_end - s_report_start >= FPS_REPORT_INTERVAL_US) {
        ESP_LOGI(TAG, "fps: %.1f, ppa: %.1f ms, flush: %.1f ms",
                 s_frames * 1000000.0f / (t_end - s_report_start),
                 s_ppa_us / 1000.0f / s_frames, s_flush_us / 1000.0f / s_frames);
        s_report_start = t_end;
        s_frames = 0;
        s_ppa_us = 0;
        s_flush_us = 0;
    }
}
