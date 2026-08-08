#include "usbHost.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_err.h"
#include "esp_log.h"

#include "usb/usb_host.h"

static const char *TAG = "USB";

static void UsbHostDaemonTask(void *arg)
{
    ESP_LOGI(TAG, "USB daemon started.");

    while (true)
    {
        uint32_t eventFlags = 0;

        esp_err_t result = usb_host_lib_handle_events(portMAX_DELAY, &eventFlags);

        if (result != ESP_OK)
        {
            ESP_LOGE(TAG,
                     "usb_host_lib_handle_events() failed (%s)",
                     esp_err_to_name(result));

            continue;
        }

        if (eventFlags & USB_HOST_LIB_EVENT_FLAGS_NO_CLIENTS)
        {
            ESP_LOGI(TAG, "USB: no registered clients.");
        }

        if (eventFlags & USB_HOST_LIB_EVENT_FLAGS_ALL_FREE)
        {
            ESP_LOGI(TAG, "USB: all devices released.");
        }
    }
}

esp_err_t UsbHost_Init(void)
{
    ESP_LOGI(TAG, "Initializing USB Host...");

    const usb_host_config_t hostConfig =
    {
        .skip_phy_setup = false,
        .intr_flags = ESP_INTR_FLAG_LEVEL1
    };

    esp_err_t result = usb_host_install(&hostConfig);

    if (result != ESP_OK)
    {
        ESP_LOGE(TAG,
                 "usb_host_install() failed (%s)",
                 esp_err_to_name(result));

        return result;
    }

    BaseType_t taskResult = xTaskCreate(
        UsbHostDaemonTask,
        "UsbHostDaemon",
        4096,
        NULL,
        5,
        NULL);

    if (taskResult != pdPASS)
    {
        ESP_LOGE(TAG, "Failed to create USB daemon task.");

        usb_host_uninstall();

        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "USB Host initialized.");

    return ESP_OK;
}