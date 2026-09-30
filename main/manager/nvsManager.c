#include <stdbool.h>
#include <stdint.h>

#include "nvs.h"
#include "nvs_flash.h"
#include "esp_log.h"

#include <nvsManager.h>

static const char *TAG = "NVS_MANAGER";

#define NVS_NAMESPACE "mistmusic"

#define KEY_TRACK  "track"
#define KEY_RANDOM "random"

static nvs_handle_t NvsHandle = 0;
static bool IsInitialized = false;

bool NvsManager_Init(void)
{
    if (IsInitialized)
        return true;

    esp_err_t err = nvs_flash_init();

    if (err == ESP_ERR_NVS_NO_FREE_PAGES ||
        err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_LOGW(TAG, "NVS requires erase");

        err = nvs_flash_erase();

        if (err != ESP_OK)
            return false;

        err = nvs_flash_init();
    }

    if (err != ESP_OK)
    {
        ESP_LOGE(
            TAG,
            "NVS initialization failed: %s",
            esp_err_to_name(err));

        return false;
    }

    err = nvs_open(
        NVS_NAMESPACE,
        NVS_READWRITE,
        &NvsHandle);

    if (err != ESP_OK)
    {
        ESP_LOGE(
            TAG,
            "Failed to open namespace: %s",
            esp_err_to_name(err));

        return false;
    }

    IsInitialized = true;

    ESP_LOGI(TAG, "NVS manager initialized");

    return true;
}

uint16_t NvsManager_GetTrack(void)
{
    uint16_t track = 1;

    if (!IsInitialized)
        return track;

    esp_err_t err =
        nvs_get_u16(NvsHandle, KEY_TRACK, &track);

    if (err == ESP_ERR_NVS_NOT_FOUND)
    {
        ESP_LOGI(
            TAG,
            "Track not found, using default: %u",
            track);

        return track;
    }

    if (err != ESP_OK)
    {
        ESP_LOGE(
            TAG,
            "Failed to read track: %s",
            esp_err_to_name(err));

        return 1;
    }

    return track;
}

bool NvsManager_SetTrack(uint16_t track)
{
    if (!IsInitialized)
        return false;

    esp_err_t err =
        nvs_set_u8(
            NvsHandle,
            KEY_TRACK,
            track);

    if (err != ESP_OK)
    {
        ESP_LOGE(
            TAG,
            "Failed to set track: %s",
            esp_err_to_name(err));

        return false;
    }

    return true;
}

bool NvsManager_GetRandom(void)
{
    uint8_t random = 0;

    if (!IsInitialized)
        return false;

    esp_err_t err =
        nvs_get_u8(
            NvsHandle,
            KEY_RANDOM,
            &random);

    if (err == ESP_ERR_NVS_NOT_FOUND)
    {
        return false;
    }

    if (err != ESP_OK)
    {
        ESP_LOGE(
            TAG,
            "Failed to read random: %s",
            esp_err_to_name(err));

        return false;
    }

    return random != 0;
}

bool NvsManager_SetRandom(bool random)
{
    if (!IsInitialized)
        return false;

    esp_err_t err =
        nvs_set_u8(
            NvsHandle,
            KEY_RANDOM,
            random ? 1 : 0);

    if (err != ESP_OK)
    {
        ESP_LOGE(
            TAG,
            "Failed to set random: %s",
            esp_err_to_name(err));

        return false;
    }

    return true;
}
bool NvsManager_Commit(void)
{
    if (!IsInitialized)
        return false;

    esp_err_t err = nvs_commit(NvsHandle);

    if (err != ESP_OK)
    {
        ESP_LOGE(
            TAG,
            "NVS commit failed: %s",
            esp_err_to_name(err));

        return false;
    }

    return true;
}