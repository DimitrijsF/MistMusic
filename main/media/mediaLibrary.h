#pragma once

#include <inttypes.h>
#include <stdbool.h>

#define MEDIA_LIBRARY_MAX_TRACKS 600

typedef enum
{
    MEDIA_SOURCE_USB,
    MEDIA_SOURCE_BLUETOOTH
} MediaSource;

typedef struct
{
    MediaSource Source;
    char Path[256];
    uint32_t Duration;
    uint32_t SampleRate;
    uint8_t Channels;
} MediaTrack;

void MediaLibrary_Print(void);
bool Media_IsSupportedFile(const char *path);
void MediaLibrary_AddTrack(MediaSource source, const char *path);
void MediaLibrary_Clear(void);
uint16_t MediaLibrary_GetCount(void);
MediaTrack *MediaLibrary_GetTrack(uint16_t number);
bool MediaLibrary_IsEmpty(void);
uint16_t MediaLibrary_GetVirtualCount(void);