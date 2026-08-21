#include "esp_log.h"

#include <dirent.h>
#include <sys/stat.h>
#include <string.h>

#include "usb/msc_host.h"
#include "usb/msc_host_vfs.h"

#include "usbStorage.h"
#include "media/mediaLibrary.h"
#include <media/mediaPlayer.h>
#include <cdc/cdc_state.h>
#include <cdc/cdc_protocol.h>

static const char *TAG = "MSC";

static msc_host_device_handle_t g_Device = NULL;
static msc_host_vfs_handle_t g_Vfs = NULL;

static volatile bool g_DeviceConnected = false;
static volatile bool g_DeviceInstalled = false;
static volatile bool g_EjectRequested = false;
static volatile bool g_DeviceDisconnectRequested = false;
static volatile uint8_t g_DeviceAddress = 0;

static void StorageCallback(const msc_host_event_t *event, void *arg)
{
    switch (event->event)
    {
        case MSC_DEVICE_CONNECTED:
            g_DeviceAddress = event->device.address;
            g_DeviceConnected = true;
            g_DeviceInstalled = false;
            g_DeviceDisconnectRequested = false;
            g_EjectRequested = false;
            ESP_LOGI(TAG,
             "MSC device connected (address=%u)",
             g_DeviceAddress);
        break;

        case MSC_DEVICE_DISCONNECTED:
            g_DeviceConnected = false;
            g_DeviceDisconnectRequested = true;
            ESP_LOGI(TAG, "MSC device disconnected");
        break;

        default:
            ESP_LOGW(TAG, "Unknown MSC event");
            break;
    }
}

static esp_err_t UsbStorage_OpenDevice(void)
{
    esp_err_t err;

    err = msc_host_install_device(g_DeviceAddress, &g_Device);

    if (err != ESP_OK)
    {
        ESP_LOGE(TAG,
                 "Failed to install device (%s)",
                 esp_err_to_name(err));

        return err;
    }

    msc_host_device_info_t info;

    err = msc_host_get_device_info(g_Device, &info);

    if (err != ESP_OK)
    {
        ESP_LOGE(TAG,
                 "Failed to read device info (%s)",
                 esp_err_to_name(err));
        g_Device = NULL;

        return err;
    }

    return ESP_OK;
}

static void UsbStorage_ScanDirectory(const char *path)
{
    DIR *dir = opendir(path);

    if (dir == NULL)
    {
        ESP_LOGE(TAG,
                 "Cannot open directory: %s",
                 path);
        return;
    }

    struct dirent *entry;

    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") == 0 ||
            strcmp(entry->d_name, "..") == 0)
        {
            continue;
        }

        char fullPath[512];

        snprintf(fullPath,
                 sizeof(fullPath),
                 "%s/%s",
                 path,
                 entry->d_name);

        struct stat st;

        if (stat(fullPath, &st) != 0)
        {
            ESP_LOGW(TAG,
                     "stat() failed: %s",
                     fullPath);
            continue;
        }

        if (S_ISDIR(st.st_mode))
        {
            UsbStorage_ScanDirectory(fullPath);
        }
        else if(Media_IsSupportedFile(fullPath))
        {
            MediaLibrary_AddTrack(MEDIA_SOURCE_USB, fullPath);
        }
    }

    closedir(dir);
}

static esp_err_t UsbStorage_ReadFS(void){
    esp_err_t err;
    esp_vfs_fat_mount_config_t mountConfig =
    {
        .format_if_mount_failed = false,
        .max_files = 5,
        .allocation_unit_size = 0
    };

    err = msc_host_vfs_register(
        g_Device,
        "/usb",
        &mountConfig,
        &g_Vfs);

    if (err != ESP_OK)
    {
        ESP_LOGE(TAG,
         "Failed to mount filesystem: %d",
         err);

        return err;
    }

    ESP_LOGI(TAG,
         "Filesystem mounted at /usb");
    UsbStorage_ScanDirectory("/usb");
    MediaLibrary_Print();
    return ESP_OK;
}

static void UsbStorageTask(void *arg){
    while (true)
    {
        if (g_EjectRequested)
        {
            g_EjectRequested = false;
            Player_Stop();
            if (g_Vfs != NULL)
            {
                msc_host_vfs_unregister(g_Vfs);
                g_Vfs = NULL;
            }
            if (g_Device != NULL)
            {
                msc_host_uninstall_device(g_Device);
                g_Device = NULL;
            }
            g_DeviceConnected = false;
            g_DeviceInstalled = false;
            g_DeviceAddress = 0;

            MediaLibrary_Clear();

            ESP_LOGI(TAG, "USB storage ejected");
        }
        if (g_DeviceDisconnectRequested)
        {
            g_DeviceDisconnectRequested = false;
            Player_Stop();

            if (g_Vfs != NULL)
            {
                msc_host_vfs_unregister(g_Vfs);
                g_Vfs = NULL;
            }

            if (g_Device != NULL)
            {
                msc_host_uninstall_device(g_Device);
                g_Device = NULL;
            }

            g_DeviceInstalled = false;
            g_DeviceAddress = 0;

            MediaLibrary_Clear();

            ESP_LOGI(TAG, "USB storage disconnected");
        }
        if (g_DeviceConnected && !g_DeviceInstalled)
        {
            g_DeviceInstalled = true;
            MediaLibrary_Clear();
            UsbStorage_OpenDevice();
            if (g_Device != NULL)
            {
                UsbStorage_ReadFS();
                if (GetCdcState() != STANDBY)
                    CdcLoadDisk();
            }
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

esp_err_t UsbStorage_Init(void)
{
    msc_host_driver_config_t config =
    {
        .create_backround_task = true,
        .task_priority = 5,
        .stack_size = 4096,
        .core_id = tskNO_AFFINITY,
        .callback = StorageCallback,
        .callback_arg = NULL
    };

    esp_err_t err = msc_host_install(&config);

    if (err != ESP_OK)
    {
        ESP_LOGE(TAG,
                 "msc_host_install() failed (%s)",
                 esp_err_to_name(err));
        return err;
    }
    xTaskCreate(
    UsbStorageTask,
    "UsbStorage",
    4096,
    NULL,
    5,
    NULL);
    ESP_LOGI(TAG, "MSC Host installed.");
    return ESP_OK;
}
void UsbStorageEject(void){
     g_EjectRequested = true;
}