#pragma once

#include <inttypes.h>
#include <stdbool.h>

#define TRACKS_PER_PAGE 99
#define VIRTUAL_TRACK_COUNT 100

typedef struct
{
    char Path[256];
    uint32_t Duration;
    uint32_t SampleRate;
    uint8_t Channels;
} UsbTrack;

bool UsbLibrary_Begin(void);
void UsbLibrary_Finish(void);
void UsbLibrary_Clear(void);

void UsbLibrary_AddTrack(const char *path);
UsbTrack *UsbLibrary_GetTrack(uint16_t number);

void UsbLibrary_SetSavedTrack(uint8_t track);
uint8_t UsbLibrary_GetSavedTrack(void);
void UsbLibrary_SetSavedPage(uint8_t page);
uint8_t UsbLibrary_GetSavedPage(void);

uint8_t UsbLibrary_GetVirtualTrack(uint16_t realTrack);
uint8_t UsbLibrary_GetVirtualPage(uint16_t realTrack);
uint32_t UsbLibrary_GetCurrentFingerprint(void);

bool UsbLibrary_IsSupportedFile(const char *path);
uint16_t UsbLibrary_GetCount(void);
bool UsbLibrary_IsEmpty(void);

uint16_t UsbLibrary_GetRealTrackByPosition(uint8_t page, uint8_t track);