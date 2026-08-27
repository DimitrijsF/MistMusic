#include <esp_log.h>
#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_netif.h>
#include <string.h>

static const char *TAG = "WIFI_CORE";

void WifiCore_Init(void){
    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());

    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    wifi_config_t wifi_config = {
        .ap = {
            .ssid = "MistMusicWF",
            .ssid_len = strlen("MistMusicWF"),
            .channel = 1,
            .password = "Pr1nc2sa!",
            .max_connection = 1,
            .authmode = WIFI_AUTH_WPA2_PSK,
        },
    };

    ESP_ERROR_CHECK(
        esp_wifi_set_mode(WIFI_MODE_AP));

    ESP_ERROR_CHECK(
        esp_wifi_set_config(WIFI_IF_AP, &wifi_config));

    ESP_ERROR_CHECK(
        esp_wifi_start());

    ESP_LOGI(TAG, "Wifi network Initialized");
}
void WifiCore_Close(void){
    ESP_ERROR_CHECK(esp_wifi_stop());
    ESP_ERROR_CHECK(esp_wifi_deinit());
    ESP_LOGI(TAG, "Wifi network stopped");
}