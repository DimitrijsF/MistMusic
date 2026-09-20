#include <powerManager.h>

#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

static const char *TAG = "POWER";

#define POWER_CONTROL_GPIO        GPIO_NUM_8

static bool IsInitialized = false;
static bool IsOn = false;

static void SetGpio(gpio_num_t gpio, int level)
{
    gpio_set_level(gpio, level);
}

void Power_Init(void)
{
    if (IsInitialized)
        return;
    gpio_config_t config = {
        .pin_bit_mask =
            (1ULL << POWER_CONTROL_GPIO),

        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    if (gpio_config(&config) != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to configure GPIO");
        return;
    }
    SetGpio(POWER_CONTROL_GPIO, 1);
    IsOn = false;
    IsInitialized = true;
    ESP_LOGI(TAG, "Power manager initialized");
    Power_Enable();
}

void Power_Enable(void)
{
    if (!IsInitialized)
        return;
    if (IsOn)
        return;
    ESP_LOGI(TAG, "Switching to external power");
    SetGpio(POWER_CONTROL_GPIO, 0);
    IsOn = true;
    ESP_LOGI(TAG, "External power active");
}

void Power_Standby(void)
{
    if (!IsInitialized || !IsOn)
        return;
    ESP_LOGI(TAG, "Switching to standby power");
    SetGpio(POWER_CONTROL_GPIO, 1);
    IsOn = false;
    ESP_LOGI(TAG, "Standby power active");
}

bool Power_IsOn(void)
{
    return IsOn;
}