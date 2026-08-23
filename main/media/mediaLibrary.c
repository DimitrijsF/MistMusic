#include <inttypes.h>
#include <stdbool.h>
#include <string.h>

#include "esp_log.h"

#include <mediaLibrary.h>

static MediaTrack g_Tracks[MEDIA_LIBRARY_MAX_TRACKS];
static uint16_t g_TrackCount = 0;
static bool IsEmpty = true;

static const char* TAG = "MEDIA_LIBRARY";

void MediaLibrary_Print(void)
{
    ESP_LOGI(TAG,
             "Tracks: %u",
             g_TrackCount);
}

bool Media_IsSupportedFile(const char *path)
{
    const char *ext = strrchr(path, '.');

    if (ext == NULL)
        return false;

    return strcasecmp(ext, ".mp3") == 0;
}

void MediaLibrary_AddTrack(MediaSource source, const char *path)
{
    if (g_TrackCount >= MEDIA_LIBRARY_MAX_TRACKS)
        return;

    MediaTrack *track =
        &g_Tracks[g_TrackCount];

    track->Source = source;

    strncpy(
        track->Path,
        path,
        sizeof(track->Path) - 1);

    track->Path[sizeof(track->Path) - 1] = '\0';

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

MediaTrack *MediaLibrary_GetTrack(uint16_t number){
    if(number - 1 > g_TrackCount || number - 1 < 0)
        return NULL;
    else{
        return &g_Tracks[number - 1];
    }
}
bool MediaLibrary_IsEmpty(void){
    return IsEmpty;
}