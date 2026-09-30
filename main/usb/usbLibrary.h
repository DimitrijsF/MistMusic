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

void UsbLibrary_Finish(void);
bool UsbLibrary_IsSupportedFile(const char *path);
void UsbLibrary_AddTrack(const char *path);
void UsbLibrary_Clear(void);
uint16_t UsbLibrary_GetCount(void);
UsbTrack *UsbLibrary_GetTrack(uint16_t number);
bool UsbLibrary_IsEmpty(void);
uint16_t UsbLibrary_GetVirtualCount(void);
void UsbLibrary_SetSavedTrack(uint8_t track);
uint8_t UsbLibrary_GetSavedTrack(void);
void UsbLibrary_SetSavedPage(uint8_t page);
uint8_t UsbLibrary_GetSavedPage(void);
bool UsbLibrary_Begin(void);