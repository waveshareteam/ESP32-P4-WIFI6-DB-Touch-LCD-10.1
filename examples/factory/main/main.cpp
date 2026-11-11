/*
 * SPDX-FileCopyrightText: 2023-2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <algorithm>
#include <atomic>
#include <cstdio>
#include <string>
#include <iterator>
#include <new>
#include <vector>
#include "esp_lv_adapter_display.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"

#include "esp_brookesia.hpp"
#include "boost/thread.hpp"
#ifdef ESP_UTILS_LOG_TAG
#   undef ESP_UTILS_LOG_TAG
#endif
#define ESP_UTILS_LOG_TAG "Main"
#include "esp_lib_utils.h"

#include "esp_system.h"
#include "esp_heap_caps.h"
#include "nvs_flash.h"

#include "board.h"
#include "board_wifi.h"
#include "SettingsStorage.hpp"
#include "BootLoading.hpp"

using namespace esp_brookesia;
using namespace esp_brookesia::gui;
using namespace esp_brookesia::systems::phone;

constexpr bool EXAMPLE_SHOW_MEM_INFO = true;
constexpr bool EXAMPLE_SHOW_MEM_INFO_LOG = false;

/*
 * Apps register themselves with ESP_UTILS_REGISTER_PLUGIN_WITH_CONSTRUCTOR and
 * are installed from the registry. This table only sets the home-screen order
 * and the boot stage shown while each app installs ("<status>: <app>..."); names
 * are the app names and progress is reached when the group is done.
 * Registered apps that are not listed here are installed after the last group.
 */
struct BootAppGroup {
    const char *status;
    int progress;
    std::vector<const char *> apps;
};

constexpr int BOOT_APP_REGISTRY_PROGRESS = 62;

static const BootAppGroup BOOT_APP_GROUPS[] = {
    {"Loading applications",        68, {"Squareline", "Calculator"}},
    {"Loading tools",               74, {"DrawPanel", "SpecAnalyzer"}},
    {"Loading media",               82, {"MusicPlayer", "Camera", "VideoPlayer"}},
    {"Loading system applications", 90, {"Settings", "AIChats", "UART", "CAN"}},
};

static std::atomic<bool> s_wifi_connected{false};

// Runs in the Wi-Fi service publisher task; only record the state for the status bar timer.
static void wifi_service_status_handler(const adf_event_t *event, void *ctx)
{
    (void)ctx;

    switch (event->event_id) {
    case BOARD_WIFI_EVENT_STA_GOT_IP:
        s_wifi_connected.store(true, std::memory_order_relaxed);
        break;
    case BOARD_WIFI_EVENT_STA_LOST_IP:
    case BOARD_WIFI_EVENT_DISCONNECTED:
        s_wifi_connected.store(false, std::memory_order_relaxed);
        break;
    default:
        break;
    }
}

static void subscribe_wifi_status_events()
{
    esp_err_t ret = board_wifi_service_subscribe(BOARD_WIFI_EVENT_ANY, wifi_service_status_handler, nullptr);
    if (ret != ESP_OK) {
        ESP_LOGW(ESP_UTILS_LOG_TAG, "Subscribe Wi-Fi service events failed: %s", esp_err_to_name(ret));
    }
}

static bool is_wifi_connected()
{
    return s_wifi_connected.load(std::memory_order_relaxed);
}

extern "C" void app_main(void)
{
    esp_log_level_set("rpc_rsp", ESP_LOG_ERROR);

    ESP_UTILS_LOGI("Reset reason: %d", static_cast<int>(esp_reset_reason()));
    ESP_UTILS_LOGI("Display ESP-Brookesia phone demo");

    lv_display_t *disp = board_display_start();
    ESP_UTILS_CHECK_NULL_EXIT(disp, "Start display failed");
    board_display_backlight_off();

    /* Configure GUI lock */
    LvLock::registerCallbacks([](int timeout_ms) {
        bool locked = board_display_lock(timeout_ms);
        ESP_UTILS_CHECK_FALSE_RETURN(locked, false, "Lock failed (timeout_ms: %d)", timeout_ms);

        return true;
    }, []() {
        board_display_unlock();
        return true;
    });

    BootLoadingUi boot_loading;
    ESP_LOGI(ESP_UTILS_LOG_TAG, "Startup stage: Preparing display... (8%%)");
    // The backlight stays off until the saved brightness has been loaded below.
    apply_and_wait_for_frame(disp, [&]() {
        if (!boot_loading.create()) {
            ESP_LOGE(ESP_UTILS_LOG_TAG, "Create boot loading UI failed");
        }
    }, BOOT_FIRST_FRAME_WAIT_MS);

    BootLoadingFailureGuard boot_failure_guard(boot_loading, disp);

    refresh_boot_loading(boot_loading, disp, "Initializing NVS...", 12);
    ESP_LOGI(ESP_UTILS_LOG_TAG, "NVS initialization begin");
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGW(ESP_UTILS_LOG_TAG, "NVS requires erase before initialization: %s", esp_err_to_name(err));
        refresh_boot_loading(boot_loading, disp, "Erasing NVS...", 13);
        err = nvs_flash_erase();
        if (err != ESP_OK) {
            ESP_LOGE(ESP_UTILS_LOG_TAG, "NVS erase failed: %s", esp_err_to_name(err));
            boot_failure_guard.setError(err);
            return;
        }
        refresh_boot_loading(boot_loading, disp, "Reinitializing NVS...", 14);
        err = nvs_flash_init();
    }
    if (err != ESP_OK) {
        ESP_LOGE(ESP_UTILS_LOG_TAG, "NVS initialization failed: %s", esp_err_to_name(err));
        boot_failure_guard.setError(err);
        return;
    }
    ESP_LOGI(ESP_UTILS_LOG_TAG, "NVS initialization complete");

    // Load the persisted parameters before any app reads them. Apps can still
    // run without the service, so a failure only disables persistence.
    refresh_boot_loading(boot_loading, disp, "Loading saved settings...", 16);
    if (!services::StorageNVS::requestInstance().begin()) {
        ESP_LOGW(ESP_UTILS_LOG_TAG, "StorageNVS service failed to start; settings will not persist");
    }

    // The "Loading saved settings..." frame was rendered above; let the panel
    // scan it out, then light the backlight at the user's saved brightness.
    vTaskDelay(pdMS_TO_TICKS(BOOT_SCANOUT_WAIT_MS));
    fade_in_backlight(apps::settings_storage::loadBrightness());

    refresh_boot_loading(boot_loading, disp, "Mounting SPIFFS storage...", 20);
    err = board_spiffs_mount();
    if (err != ESP_OK) {
        ESP_LOGE(ESP_UTILS_LOG_TAG, "SPIFFS mount failed: %s", board_diag_err_to_name(err));
        boot_failure_guard.setError(err, board_diag_failed_device());
        return;
    }
    ESP_LOGI(ESP_UTILS_LOG_TAG, "SPIFFS mount successfully");

    refresh_boot_loading(boot_loading, disp, "Starting audio codec...", 28);
    err = board_codec_init();
    if (err != ESP_OK) {
        ESP_LOGE(ESP_UTILS_LOG_TAG, "Codec initialization failed: %s", board_diag_err_to_name(err));
        boot_failure_guard.setError(err, board_diag_failed_device());
        return;
    }

    refresh_boot_loading(boot_loading, disp, "Setting audio volume...", 45);
    err = board_codec_volume_set(apps::settings_storage::loadVolume(), NULL);
    if (err != ESP_OK) {
        ESP_LOGE(ESP_UTILS_LOG_TAG, "Codec volume setup failed: %s", board_diag_err_to_name(err));
        boot_failure_guard.setError(err, board_diag_failed_device());
        return;
    }

    /* Create a phone object */
    refresh_boot_loading(boot_loading, disp, "Creating phone system...", 52);
    PhoneStartupOwner phone_owner(new (std::nothrow) Phone(disp), boot_loading, disp);
    Phone *phone = phone_owner.get();
    ESP_UTILS_CHECK_NULL_EXIT(phone, "Create phone failed");

    refresh_boot_loading(boot_loading, disp, "Applying display stylesheet...", 55);
    {
        // When operating on non-GUI tasks, should acquire a lock before operating on LVGL
        LvLockGuard gui_guard;
        const int width = lv_disp_get_hor_res(disp);
        const int height = lv_disp_get_ver_res(disp);
        // Selected at compile time so only one stylesheet is linked in. The layout symbol
        // comes from the DSI panel amend in esp32_p4_dsi_overlay and names the panel's native
        // resolution, which board_display shows unrotated. Without an amend the board's own
        // board_devices.yaml applies, and that is the 800x1280 JD9365.
#if defined(CONFIG_BOARD_DISPLAY_LAYOUT_1024_600)
        const auto *target_stylesheet = &ESP_BROOKESIA_PHONE_1024_600_DARK_STYLESHEET();
#elif defined(CONFIG_BOARD_DISPLAY_LAYOUT_720_1280)
        const auto *target_stylesheet = &ESP_BROOKESIA_PHONE_720_1280_DARK_STYLESHEET();
#else
        const auto *target_stylesheet = &ESP_BROOKESIA_PHONE_800_1280_DARK_STYLESHEET();
#endif

        ESP_LOGI(ESP_UTILS_LOG_TAG, "Display resolution: %dx%d; stylesheet: %s", width, height, target_stylesheet->core.name);
        ESP_UTILS_CHECK_FALSE_EXIT(phone->addStylesheet(*target_stylesheet), "Add display stylesheet failed");
        ESP_UTILS_CHECK_FALSE_EXIT(phone->activateStylesheet(*target_stylesheet), "Activate display stylesheet failed");
    }

    /* Begin the phone */
    refresh_boot_loading(boot_loading, disp, "Building home screen...", 58);
    {
        LvLockGuard gui_guard;
        ESP_UTILS_CHECK_FALSE_EXIT(phone->begin(), "Begin failed");
        boot_loading.attachToSystemLayer(disp);
        // assert(phone->getDisplay().showContainerBorder() && "Show container border failed");
    }

    refresh_boot_loading(boot_loading, disp, "Loading application registry...", BOOT_APP_REGISTRY_PROGRESS);
    std::vector<systems::base::Manager::RegistryAppInfo> registry_apps;
    auto &app_manager = phone->getManager();
    {
        LvLockGuard gui_guard;
        ESP_UTILS_CHECK_FALSE_EXIT(app_manager.initAppFromRegistry(registry_apps), "Init app registry failed");
    }

    auto install = [&](const char *group_status, const std::string & name, systems::base::App * app, int progress) {
        char stage[64];
        snprintf(stage, sizeof(stage), "%s: %s...", group_status, name.c_str());
        refresh_boot_loading(boot_loading, disp, stage, progress);

        // Creating and installing apps builds home-screen widgets; hold the LVGL lock.
        LvLockGuard gui_guard;
        ESP_UTILS_LOGI("Install app: %s", name.c_str());
        return app_manager.checkAppID_Valid(app_manager.installApp(app));
    };

    /* Install registered apps group by group, in BOOT_APP_GROUPS order. */
    int group_start_progress = BOOT_APP_REGISTRY_PROGRESS;
    for (const BootAppGroup &group : BOOT_APP_GROUPS) {
        const int app_count = static_cast<int>(group.apps.size());
        for (int app_index = 0; app_index < app_count; ++app_index) {
            const char *app_name = group.apps[app_index];
            auto it = std::find_if(registry_apps.begin(), registry_apps.end(), [app_name](const auto & info) {
                return std::get<0>(info) == app_name;
            });
            if (it == registry_apps.end()) {
                ESP_UTILS_LOGW("App %s is not registered; skip it", app_name);
                continue;
            }
            const int progress = group_start_progress + (group.progress - group_start_progress) * (app_index + 1) / app_count;
            ESP_UTILS_CHECK_FALSE_EXIT(install(group.status, std::get<0>(*it), std::get<1>(*it).get(), progress),
                                       "Install app %s failed", app_name);
            registry_apps.erase(it);
        }
        group_start_progress = group.progress;
    }

    const BootAppGroup &last_group = BOOT_APP_GROUPS[std::size(BOOT_APP_GROUPS) - 1];
    for (const auto &[name, app] : registry_apps) {
        ESP_UTILS_CHECK_FALSE_EXIT(install(last_group.status, name, app.get(), last_group.progress),
                                   "Install app %s failed", name.c_str());
    }
    registry_apps.clear();

    // Apps subscribed to Wi-Fi service events during install; start Wi-Fi last so it does not delay the UI.
    refresh_boot_loading(boot_loading, disp, "Starting Wi-Fi...", 93);
    subscribe_wifi_status_events();
    err = board_wifi_service_init();
    if (err != ESP_OK) {
        // Keep booting without networking.
        ESP_LOGE(ESP_UTILS_LOG_TAG, "Wi-Fi service initialization failed: %s", esp_err_to_name(err));
    }
    // The service may already have an address (e.g. the init call was a no-op).
    if (board_wifi_service_is_connected()) {
        s_wifi_connected.store(true, std::memory_order_relaxed);
    }

    refresh_boot_loading(boot_loading, disp, "Starting status bar...", 96);

    {
        LvLockGuard gui_guard;

        /* Create a timer to update the clock */
        lv_timer_t *status_timer = lv_timer_create([](lv_timer_t *t) {
            time_t now;
            struct tm timeinfo;
            Phone *phone = (Phone *)t->user_data;

            ESP_UTILS_CHECK_NULL_EXIT(phone, "Invalid phone");

            time(&now);
            localtime_r(&now, &timeinfo);

            ESP_UTILS_CHECK_FALSE_EXIT(
                phone->getDisplay().getStatusBar()->setClock(timeinfo.tm_hour, timeinfo.tm_min),
                "Refresh status bar failed"
            );

            bool connected = is_wifi_connected();
            StatusBar::WifiState wifi_state = connected ? StatusBar::WifiState::SIGNAL_3 : StatusBar::WifiState::DISCONNECTED;

            ESP_UTILS_CHECK_FALSE_EXIT(
                phone->getDisplay().getStatusBar()->setWifiIconState(wifi_state),
                "Refresh status bar failed"
            );
        }, 1000, phone);
        ESP_UTILS_CHECK_NULL_EXIT(status_timer, "Create status timer failed");

        phone_owner.release();
        boot_failure_guard.dismiss();
        // Keep the completed state visible briefly, then reveal the home screen smoothly.
        boot_loading.finish();
    }

    if constexpr(EXAMPLE_SHOW_MEM_INFO) {
        esp_utils::thread_config_guard thread_config({
            .name = "mem_info",
            .stack_size = 4096,
        });
        boost::thread([ = ]() {
            char buffer[128];    /* Make sure buffer is enough for `sprintf` */
            size_t internal_free = 0;
            size_t internal_total = 0;
            size_t external_free = 0;
            size_t external_total = 0;

            while (1) {
                internal_free = heap_caps_get_free_size(MALLOC_CAP_INTERNAL);
                internal_total = heap_caps_get_total_size(MALLOC_CAP_INTERNAL);
                external_free = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
                external_total = heap_caps_get_total_size(MALLOC_CAP_SPIRAM);
                if (EXAMPLE_SHOW_MEM_INFO_LOG) {
                    snprintf(
                        buffer,
                        sizeof(buffer),
                        "\t           Biggest /     Free /    Total\n"
                        "\t  SRAM : [%8zu / %8zu / %8zu]\n"
                        "\t PSRAM : [%8zu / %8zu / %8zu]",
                        heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL), internal_free, internal_total,
                        heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM), external_free, external_total
                    );
                    ESP_UTILS_LOGI("\n%s", buffer);
                }
                {
                    LvLockGuard gui_guard;
                    ESP_UTILS_CHECK_FALSE_EXIT(
                        phone->getDisplay().getRecentsScreen()->setMemoryLabel(
                            internal_free / 1024, internal_total / 1024, external_free / 1024, external_total / 1024
                        ), "Set memory label failed"
                    );
                }

                boost::this_thread::sleep_for(boost::chrono::seconds(5));
            }
        }).detach();
    }
}
