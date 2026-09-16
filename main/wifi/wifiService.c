#include <stdbool.h>
#include <string.h>

#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"

#include "wifiService.h"
#include "wifiHttp.h"

static const char *TAG = "WIFI";

static bool s_initialized = false;


void WifiService_Init(void)
{
    if (s_initialized)
        return;

    ESP_ERROR_CHECK(
        esp_netif_init()
    );

    ESP_ERROR_CHECK(
        esp_event_loop_create_default()
    );

    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

    ESP_ERROR_CHECK(
        esp_wifi_init(&cfg)
    );

    wifi_config_t wifi_config = {
        .ap = {
            .ssid = "MistMusic",
            .ssid_len = strlen("MistMusic"),
            .channel = 1,
            .password = "password", //change before flash
            .max_connection = 1,
            .authmode = WIFI_AUTH_WPA2_PSK
        }
    };

    ESP_ERROR_CHECK(
        esp_wifi_set_mode(WIFI_MODE_AP)
    );

    ESP_ERROR_CHECK(
        esp_wifi_set_config(WIFI_IF_AP, &wifi_config)
    );

    s_initialized = true;

    ESP_LOGI(TAG, "WiFi service initialized");
}


void WifiService_Start(void)
{
    if (!s_initialized)
        return;

    ESP_ERROR_CHECK(
        esp_wifi_start()
    );

    WifiHttp_Start();

    ESP_LOGI(TAG, "Access Point started");
}


void WifiService_Stop(void)
{
    if (!s_initialized)
        return;

    WifiHttp_Stop();

    ESP_ERROR_CHECK(
        esp_wifi_stop()
    );

    ESP_LOGI(TAG, "Access Point stopped");
}