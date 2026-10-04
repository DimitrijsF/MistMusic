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
#define KEY_DISKIN "diskIn"
#define KEY_FINGERPRINT "fingerprint"
#define KEY_SECONDS "playedSeconds"
#define KEY_POSITION "trackPosition"

static nvs_handle_t NvsHandle = 0;
static bool IsInitialized = false;

bool NvsManager_Init(void){
    if (IsInitialized)
        return true;
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND){
        ESP_LOGW(TAG, "NVS requires erase");
        err = nvs_flash_erase();
        if (err != ESP_OK)
            return false;
        err = nvs_flash_init();
    }
    if (err != ESP_OK){
        ESP_LOGE(TAG, "NVS initialization failed: %s", esp_err_to_name(err));
        return false;
    }
    err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &NvsHandle);
    if (err != ESP_OK){
        ESP_LOGE(TAG, "Failed to open namespace: %s", esp_err_to_name(err));
        return false;
    }
    IsInitialized = true;
    ESP_LOGI(TAG, "NVS manager initialized");
    return true;
}
uint16_t NvsManager_GetTrack(void){
    uint16_t track = 0;
    if (!IsInitialized)
        return track;
    esp_err_t err = nvs_get_u16(NvsHandle, KEY_TRACK, &track);
    if (err == ESP_ERR_NVS_NOT_FOUND){
        ESP_LOGI(TAG, "Track not found, using default: %u", track);
        return track;
    }
    if (err != ESP_OK){
        ESP_LOGE(TAG, "Failed to read track: %s", esp_err_to_name(err));
        return 0;
    }
    return track;
}
bool NvsManager_SetTrack(uint16_t track){
    if (!IsInitialized)
        return false;
    esp_err_t err = nvs_set_u16(NvsHandle, KEY_TRACK, track);
    if (err != ESP_OK){
        ESP_LOGE(TAG, "Failed to set track: %s", esp_err_to_name(err));
        return false;
    }
    return true;
}
uint32_t NvsManager_GetResumeSeconds(void){
    uint32_t seconds = 0;
    if (!IsInitialized)
        return seconds;
    esp_err_t err = nvs_get_u32(NvsHandle, KEY_SECONDS, &seconds);
    if (err == ESP_ERR_NVS_NOT_FOUND){
        ESP_LOGI(TAG, "Seconds not found, using default: %" PRIu32, seconds);
        return seconds;
    }
    if (err != ESP_OK){
        ESP_LOGE(TAG, "Failed to read resume seconds: %s", esp_err_to_name(err));
        return 0;
    }
    return seconds;
}
bool NvsManager_SetResumeSeconds(uint32_t seconds){
    if (!IsInitialized)
        return false;
    esp_err_t err = nvs_set_u32(NvsHandle, KEY_SECONDS, seconds);
    if (err != ESP_OK){
        ESP_LOGE(TAG, "Failed to set resume seconds: %s", esp_err_to_name(err));
        return false;
    }
    return true;
}
long NvsManager_GetResumePosition(void)
{
    long position = 0;
    if (!IsInitialized)
        return position;
    int64_t nvsPosition = 0;
    esp_err_t err = nvs_get_i64(NvsHandle, KEY_POSITION, &nvsPosition);
    if (err == ESP_ERR_NVS_NOT_FOUND){
        ESP_LOGI(TAG, "Resume position not found, using default: %ld", position);
        return position;
    }
    if (err != ESP_OK){
        ESP_LOGE(TAG, "Failed to read position: %s", esp_err_to_name(err));
        return 0;
    }
    position = (long)nvsPosition;
    return position;
}
bool NvsManager_SetResumePosition(long position){
    if (!IsInitialized)
        return false;
    esp_err_t err = nvs_set_i64(NvsHandle, KEY_POSITION, position);
    if (err != ESP_OK){
        ESP_LOGE(TAG, "Failed to set position: %s", esp_err_to_name(err));
        return false;
    }
    return true;
}
bool NvsManager_GetRandom(void){
    uint8_t random = 0;
    if (!IsInitialized)
        return false;
    esp_err_t err = nvs_get_u8(NvsHandle, KEY_RANDOM, &random);
    if (err == ESP_ERR_NVS_NOT_FOUND){
        return false;
    }
    if (err != ESP_OK){
        ESP_LOGE(TAG, "Failed to read random: %s", esp_err_to_name(err));
        return false;
    }
    return random != 0;
}
bool NvsManager_SetRandom(bool random){
    if (!IsInitialized)
        return false;
    esp_err_t err = nvs_set_u8(NvsHandle, KEY_RANDOM, random ? 1 : 0);
    if (err != ESP_OK){
        ESP_LOGE(TAG, "Failed to set random: %s", esp_err_to_name(err));
        return false;
    }
    return true;
}
bool NvsManager_GetDiskIn(void){
    uint8_t diskIn = 0;
    if (!IsInitialized)
        return false;
    esp_err_t err = nvs_get_u8(NvsHandle, KEY_DISKIN, &diskIn);
    if (err == ESP_ERR_NVS_NOT_FOUND){
        return false;
    }
    if (err != ESP_OK){
        ESP_LOGE(TAG, "Failed to read diskIn: %s", esp_err_to_name(err));
        return false;
    }
    return diskIn != 0;
}
bool NvsManager_SetDiskIn(bool diskIn){
    if (!IsInitialized)
        return false;
    esp_err_t err = nvs_set_u8(NvsHandle, KEY_DISKIN, diskIn ? 1 : 0);
    if (err != ESP_OK){
        ESP_LOGE(TAG, "Failed to set diskIn: %s", esp_err_to_name(err));
        return false;
    }
    return true;
}
uint32_t NvsManager_GetFingerprint(void){
    uint32_t fingerprint = 0;
    if (IsInitialized){
        esp_err_t err = nvs_get_u32(NvsHandle, KEY_FINGERPRINT, &fingerprint);
        if (err == ESP_ERR_NVS_NOT_FOUND){
            ESP_LOGI(TAG, "Fingerprint not found, using empty");
            return fingerprint;
        }
        if (err != ESP_OK){
            ESP_LOGE(TAG, "Failed to read track: %s", esp_err_to_name(err));
            return fingerprint;
        }
    }
    return fingerprint;
}
bool NvsManager_SetFingerprint(uint32_t fingerprint){
     if (!IsInitialized)
        return false;
    esp_err_t err = nvs_set_u32(NvsHandle, KEY_FINGERPRINT, fingerprint);
    if (err != ESP_OK){
        ESP_LOGE(TAG, "Failed to set fingerprint: %s", esp_err_to_name(err));
        return false;
    }
    return true;
}
bool NvsManager_Commit(void){
    if (!IsInitialized)
        return false;
    esp_err_t err = nvs_commit(NvsHandle);
    if (err != ESP_OK){
        ESP_LOGE(TAG, "NVS commit failed: %s", esp_err_to_name(err));
        return false;
    }
    return true;
}