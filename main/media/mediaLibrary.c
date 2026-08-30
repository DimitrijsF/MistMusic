#include <inttypes.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include "esp_log.h"

#include <mediaLibrary.h>
#include <media/mediaPlayer.h>

#define MEDIA_LIBRARY_INDEX_PATH "/usb/cd30_index"

static uint16_t g_TrackCount = 0;
static bool IsEmpty = true;
static uint8_t SavedTrack = 0;
static uint8_t SavedPage = 0;

static const char* TAG = "MEDIA_LIBRARY";
static FILE *g_IndexFile = NULL;

static MediaTrack g_CurrentTrack;
static uint32_t g_CurrentCrc = 0xFFFFFFFF;
static uint32_t Fingerprint = 0;

static uint32_t MediaLibrary_UpdateCrc32(uint32_t crc, const uint8_t *data, size_t length);

bool MediaLibrary_Begin(void)
{
    g_TrackCount = 0;
    IsEmpty = true;

    g_CurrentCrc = 0xFFFFFFFF;

    g_IndexFile =
        fopen(
            MEDIA_LIBRARY_INDEX_PATH,
            "wb");

    if(g_IndexFile == NULL)
    {
        ESP_LOGE(
            TAG,
            "Cannot create index");

        return false;
    }

    return true;
}
void MediaLibrary_Finish(void)
{
    if(g_IndexFile != NULL)
    {
        fclose(g_IndexFile);
        g_IndexFile = NULL;
    }

    uint32_t currentPrint =
        g_CurrentCrc ^ 0xFFFFFFFF;

    ESP_LOGI(
        TAG,
        "Tracks: %u",
        g_TrackCount);

    if(Fingerprint != 0)
    {
        if(currentPrint != Fingerprint)
        {
            ESP_LOGI(
                TAG,
                "Library changed");

            SavedTrack = 0;
            SavedPage = 0;
        }
    }

    Fingerprint = currentPrint;
}

bool Media_IsSupportedFile(const char *path)
{
    const char *ext = strrchr(path, '.');

    if (ext == NULL)
        return false;

    return strcasecmp(ext, ".mp3") == 0;
}

void MediaLibrary_AddTrack(
    MediaSource source,
    const char *path)
{
    if(g_IndexFile == NULL)
        return;

    MediaTrack track = {0};

    track.Source = source;

    strncpy(
        track.Path,
        path,
        sizeof(track.Path) - 1);

    g_CurrentCrc =
        MediaLibrary_UpdateCrc32(
            g_CurrentCrc,
            (const uint8_t *)&track.Source,
            sizeof(track.Source));

    g_CurrentCrc =
        MediaLibrary_UpdateCrc32(
            g_CurrentCrc,
            (const uint8_t *)track.Path,
            strlen(track.Path));

    const uint8_t separator = 0;

    g_CurrentCrc =
        MediaLibrary_UpdateCrc32(
            g_CurrentCrc,
            &separator,
            sizeof(separator));

    if(fwrite(
        &track,
        sizeof(MediaTrack),
        1,
        g_IndexFile) != 1)
    {
        ESP_LOGE(
            TAG,
            "Failed to write index");

        return;
    }

    g_TrackCount++;
    IsEmpty = false;
}

void MediaLibrary_Clear(void){
    g_TrackCount = 0;
    IsEmpty = true;
}

uint16_t MediaLibrary_GetCount(void){
    return g_TrackCount;
}
uint16_t MediaLibrary_GetVirtualCount(void){
    if(g_TrackCount >= TRACKS_PER_PAGE)
        return SWITCH_TRACK_NUMBER;
    else 
        return g_TrackCount;
}

MediaTrack *MediaLibrary_GetTrack(uint16_t number){
    if(number == 0 ||
       number > g_TrackCount)
    {
        return NULL;
    }

    FILE *file =
        fopen(
            MEDIA_LIBRARY_INDEX_PATH,
            "rb");

    if(file == NULL)
    {
        ESP_LOGE(
            TAG,
            "Cannot open index");

        return NULL;
    }

    long offset =
        (long)(number - 1) *
        sizeof(MediaTrack);

    if(fseek(
        file,
        offset,
        SEEK_SET) != 0)
    {
        fclose(file);
        return NULL;
    }

    if(fread(
        &g_CurrentTrack,
        sizeof(MediaTrack),
        1,
        file) != 1)
    {
        fclose(file);
        return NULL;
    }

    fclose(file);

    return &g_CurrentTrack;
}
bool MediaLibrary_IsEmpty(void){
    return IsEmpty;
}
static uint32_t MediaLibrary_UpdateCrc32(
    uint32_t crc,
    const uint8_t *data,
    size_t length)
{
    for(size_t i = 0; i < length; i++)
    {
        crc ^= data[i];

        for(uint8_t bit = 0; bit < 8; bit++)
        {
            if(crc & 1)
                crc = (crc >> 1) ^ 0xEDB88320;
            else
                crc >>= 1;
        }
    }
    return crc;
}
void MediaLibrary_SetSavedTrack(uint8_t track){
    SavedTrack = track;
}
uint8_t MediaLibrary_GetSavedTrack(void){
    return SavedTrack;
}
void MediaLibrary_SetSavedPage(uint8_t page){
    SavedPage = page;
}
uint8_t MediaLibrary_GetSavedPage(void){
    return SavedPage;
}