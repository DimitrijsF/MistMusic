#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "esp_log.h"
#include "esp_heap_caps.h"

#include <usbBuffer.h>
#include <usbStorage.h>

static const char *TAG = "USB_INDEX_BUFFER";

UsbTrackIndexBuffer mainBuffer = {0};
UsbTrackIndexBuffer plstBuffer = {0};

void UsbBuffer_Init(UsbTrackIndexBuffer *buffer){
    if (buffer == NULL)
        return;

    buffer->Entries = NULL;
    buffer->Count = 0;
    buffer->Capacity = 0;
}

static bool UsbBuffer_Reserve(UsbTrackIndexBuffer *buffer, size_t requiredCapacity){
    if (buffer == NULL)
        return false;

    if (requiredCapacity <= buffer->Capacity)
        return true;

    if (requiredCapacity > SIZE_MAX / sizeof(UsbTrack)){
        ESP_LOGE(TAG, "Index buffer size overflow");
        return false;
    }
    size_t newCapacity = buffer->Capacity;
    while (newCapacity < requiredCapacity){
        if (newCapacity > SIZE_MAX - USB_INDEX_BUFFER_GROWTH_STEP){
            ESP_LOGE(TAG, "Index buffer capacity overflow");
            return false;
        }
        newCapacity += USB_INDEX_BUFFER_GROWTH_STEP;
    }
    size_t newSize = newCapacity * sizeof(UsbTrack);
    UsbTrack *newEntries;
    if (buffer->Entries == NULL){
        newEntries = heap_caps_malloc(newSize, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    }
    else{
        newEntries = heap_caps_realloc(buffer->Entries, newSize, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    }

    if (newEntries == NULL){
        ESP_LOGE(TAG, "Cannot allocate %u bytes for index buffer", (unsigned int)newSize);
        return false;
    }
    buffer->Entries = newEntries;
    buffer->Capacity = newCapacity;
    return true;
}

bool UsbBuffer_Add(UsbTrackIndexBuffer *buffer, const UsbTrack *track){
    if (buffer == NULL || track == NULL)
        return false;

    if (buffer->Count == SIZE_MAX)
        return false;

    if (!UsbBuffer_Reserve(buffer, buffer->Count + 1))
        return false;

    buffer->Entries[buffer->Count] = *track;
    buffer->Count++;

    return true;
}
bool UsbBuffer_Save(const UsbTrackIndexBuffer *buffer, const char *path){
    if (buffer == NULL || path == NULL) {
        ESP_LOGE(TAG, "Invalid arguments for index saving");
        return false;
    }

    if (buffer->Count > buffer->Capacity ||
        (buffer->Count > 0 && buffer->Entries == NULL)) {
        ESP_LOGE(TAG, "Invalid index buffer state");
        return false;
    }

    ESP_LOGI(TAG, "Opening playlist file for writing");
    FILE *file = fopen(path, "wb");

    if (file == NULL){
        ESP_LOGE(TAG, "Cannot open index for writing: %s", path);
        return false;
    }
    ESP_LOGI(TAG, "Playlist file for writing is opened");
    size_t written = 0;

    if (buffer->Count > 0){
        written = fwrite(buffer->Entries, sizeof(UsbTrack), buffer->Count, file);
    }

    bool success = written == buffer->Count;

    if (fflush(file) != 0)
        success = false;

    if (fclose(file) != 0)
        success = false;

    if (!success){
        ESP_LOGE(TAG, "Failed to write index: %s", path);
        return false;
    }
    ESP_LOGI(TAG, "Index saved: %s, records: %u", path, (unsigned int)buffer->Count);

    return true;
}

void UsbBuffer_Free(UsbTrackIndexBuffer *buffer){
    if (buffer == NULL)
        return;

    free(buffer->Entries);

    buffer->Entries = NULL;
    buffer->Count = 0;
    buffer->Capacity = 0;
}