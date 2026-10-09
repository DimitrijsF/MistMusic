#pragma once

#include <inttypes.h>
#include <stdbool.h>

#define TRACKS_PER_PAGE 99
#define VIRTUAL_TRACK_COUNT 100

typedef struct{
    char Path[256];
    uint32_t Duration;
    uint32_t SampleRate;
    uint8_t Channels;
} UsbTrack;

typedef enum{
    MEDIA,
    PLAYLIST,
    OTHER
} UsbFileType;

typedef struct
{
    char name[64];
    char playlistPath[128];
    char indexPath[128];
    uint32_t trackCount;
} UsbPlaylist;

bool UsbLibrary_Begin(void);
void UsbLibrary_Finish(void);
void UsbLibrary_Clear(void);

UsbTrack *UsbLibrary_GetTrack(uint16_t number);
const UsbPlaylist *UsbLibrary_GetPlaylists(void);
uint16_t UsbLibrary_GetPlaylistCount(void);
bool UsbLibrary_SetCurrentPlayList(uint8_t index);
void UsbLibrary_SetAllTracks(void);
uint16_t UsbLibrary_GetCurrentPlaylistIndex(void);
UsbPlaylist *UsbLibrary_GetCurrentPlaylist(void);
void UsbLibrary_SetCurrentPlaylistByName(const char *name);

void UsbLibrary_SetSavedTrack(uint8_t track);
uint8_t UsbLibrary_GetSavedTrack(void);
void UsbLibrary_SetSavedPage(uint8_t page);
uint8_t UsbLibrary_GetSavedPage(void);

uint8_t UsbLibrary_GetVirtualTrack(uint16_t realTrack);
uint8_t UsbLibrary_GetVirtualPage(uint16_t realTrack);
uint32_t UsbLibrary_GetCurrentFingerprint(void);

void UsbLibrary_ProcessFile(const char *path);
uint16_t UsbLibrary_GetCount(void);
bool UsbLibrary_IsEmpty(void);

uint16_t UsbLibrary_GetRealTrackByPosition(uint8_t page, uint8_t track);