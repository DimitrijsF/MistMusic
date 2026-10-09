#include <inttypes.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include "esp_log.h"
#include <manager/playerManager.h>
#include <manager/nvsManager.h>

#include <usbLibrary.h>
#include <usbPlayer.h>
#include <usbStorage.h>

#define MEDIA_LIBRARY_INDEX_PATH "/usb/cd30_index"
#define USB_LIBRARY_MAX_PLAYLISTS 32

static const char* TAG = "USB_LIBRARY";

static FILE *IndexFile = NULL;
static uint32_t CurrentCrc = 0xFFFFFFFF;
static bool IsEmpty = true;
static uint8_t SavedTrack = 0;
static uint8_t SavedPage = 0;

static UsbTrack CurrentTrack;
static uint16_t TotalTrackCount = 0;

static UsbPlaylist PlayLists[USB_LIBRARY_MAX_PLAYLISTS];
static uint16_t PlayListCount = 0;

static UsbPlaylist *CurrentPlaylist = NULL;

static uint32_t UsbLibrary_UpdateCrc32(uint32_t crc, const uint8_t *data, size_t length);
static void UsbLibrary_AddTrack(const char *path);
static void UsbLibrary_AddPlayList(const char *path);
static UsbFileType UsbLibrary_GetFileType(const char *path);

bool UsbLibrary_Begin(void)
{
    TotalTrackCount = 0;
    IsEmpty = true;
    CurrentCrc = 0xFFFFFFFF;
    IndexFile = fopen(MEDIA_LIBRARY_INDEX_PATH, "wb");
    if(IndexFile == NULL){
        ESP_LOGE(TAG, "Cannot create index");
        return false;
    }
    return true;
}
void UsbLibrary_Finish(void){
    if(IndexFile != NULL){
        fclose(IndexFile);
        IndexFile = NULL;
    }
    ESP_LOGI(TAG, "Track count %u", TotalTrackCount);
    uint32_t currentPrint = CurrentCrc ^ 0xFFFFFFFF;
    uint32_t fingerprint = NvsManager_GetFingerprint();
    if(fingerprint != 0){
        if(currentPrint != fingerprint){
            ESP_LOGI(TAG, "Library changed");
            SavedTrack = 0;
            SavedPage = 0;
        }
        else
            UsbPlayer_SetResumeState();
    }
}

void UsbLibrary_ProcessFile(const char *path){
    UsbFileType type = UsbLibrary_GetFileType(path);
    switch (type){
        case MEDIA:
            UsbLibrary_AddTrack(path);
            break;
        case PLAYLIST:
            UsbLibrary_AddPlayList(path);
            break;
        case OTHER: default:
            return;
    }
}
static void UsbLibrary_AddPlayList(const char *path){
    FILE *file = UsbStorage_OpenFile(path);
    if (file == NULL){
        ESP_LOGW(TAG, "Cannot open playlist: %s", path);
        return;
    }

    if (PlayListCount >= USB_LIBRARY_MAX_PLAYLISTS){
        ESP_LOGW(TAG, "Playlist limit reached");
        fclose(file);
        return;
    }
    const char *fileName = strrchr(path, '/');
    fileName = (fileName != NULL) ? fileName + 1 : path;
    const char *ext = strrchr(fileName, '.');
    if (ext == NULL){
        ESP_LOGW(TAG, "Playlist has no extension: %s", path);
        fclose(file);
        return;
    }
    UsbPlaylist list = {0};
    size_t nameLength = (size_t)(ext - fileName);
    if (nameLength == 0 || nameLength >= sizeof(list.name)){
        ESP_LOGW(TAG, "Invalid playlist name: %s", path);
        fclose(file);
        return;
    }
    memcpy(list.name, fileName, nameLength);
    list.name[nameLength] = '\0';
    int result = snprintf(
        list.indexPath,
        sizeof(list.indexPath),
        "%.*s.idx",
        (int)(ext - path),
        path);
    if (result < 0 || (size_t)result >= sizeof(list.indexPath)){
        ESP_LOGW(TAG, "Playlist index path is too long: %s", path);
        fclose(file);
        return;
    }
    FILE *plstIndex = fopen(list.indexPath, "wb");
    if (plstIndex == NULL){
        ESP_LOGE(TAG, "Cannot create playlist index: %s", list.indexPath);
        fclose(file);
        return;
    }
    char line[sizeof(((UsbTrack *)0)->Path)];
    bool success = true;
    while (true){
        if (fgets(line, sizeof(line), file) == NULL){
            if (ferror(file)){
                ESP_LOGE(TAG, "Failed to read playlist: %s", path);
                success = false;
            }
            break;
        }
        size_t length = strlen(line);

        // Если строка не поместилась целиком, пропускаем её.
        if (length > 0 && line[length - 1] != '\n' && !feof(file)){
            int ch;
            while ((ch = fgetc(file)) != '\n' && ch != EOF){}
            ESP_LOGW(TAG, "Playlist entry is too long: %s", path);
            continue;
        }

        line[strcspn(line, "\r\n")] = '\0';
        if (line[0] == '\0')
            continue;
        if (UsbLibrary_GetFileType(line) != MEDIA)
            continue;
        if (!UsbStorage_FileExists(line))
            continue;
        UsbTrack track = {0};
        strncpy(track.Path, line, sizeof(track.Path) - 1);

        if (fwrite(&track, sizeof(UsbTrack), 1, plstIndex) != 1){
            ESP_LOGE(TAG, "Failed to write playlist index: %s", list.indexPath);
            success = false;
            break;
        }
        list.trackCount++;
    }

    if (fclose(plstIndex) != 0){
        ESP_LOGE(TAG, "Failed to close playlist index: %s", list.indexPath);
        success = false;
    }
    fclose(file);
    if (!success){
        remove(list.indexPath);
        return;
    }
    if (list.trackCount == 0){
        ESP_LOGW(TAG, "Playlist contains no valid tracks: %s", path);
        remove(list.indexPath);
        return;
    }
    PlayLists[PlayListCount] = list;
    PlayListCount++;
    ESP_LOGI(
        TAG,
        "Playlist added: %s, tracks: %lu",
        list.name,
        (unsigned long)list.trackCount);
}
static void UsbLibrary_AddTrack(const char *path)
{
    if(IndexFile == NULL)
        return;
    UsbTrack track = {0};
    strncpy(track.Path, path, sizeof(track.Path) - 1);
    CurrentCrc = UsbLibrary_UpdateCrc32(CurrentCrc, (const uint8_t *)track.Path, strlen(track.Path));
    const uint8_t separator = 0;
    CurrentCrc = UsbLibrary_UpdateCrc32(CurrentCrc, &separator, sizeof(separator));
    if(fwrite(&track, sizeof(UsbTrack), 1, IndexFile) != 1){
        ESP_LOGE(TAG, "Failed to write index");
        return;
    }
    TotalTrackCount++;
    IsEmpty = false;
}

void UsbLibrary_Clear(void){
    TotalTrackCount = 0;
    IsEmpty = true;
    CurrentPlaylist = NULL;
    PlayListCount = 0;
}

uint16_t UsbLibrary_GetCount(void){
    if(CurrentPlaylist != NULL)
         return CurrentPlaylist->trackCount;

    return TotalTrackCount;
}

UsbTrack *UsbLibrary_GetTrack(uint16_t number){
    if(number == 0 || (CurrentPlaylist == NULL && number > TotalTrackCount) || (CurrentPlaylist != NULL && number > CurrentPlaylist -> trackCount))
        number = 1;

    FILE *index;
    if(CurrentPlaylist == NULL)
    {
        index = fopen(MEDIA_LIBRARY_INDEX_PATH, "rb");
        ESP_LOGI(TAG, "Opening library index %s", MEDIA_LIBRARY_INDEX_PATH);
    }
    else
    {
        index = fopen(CurrentPlaylist -> indexPath, "rb");
        ESP_LOGI(TAG, "Opening playlist index %s", CurrentPlaylist -> indexPath);
    }
    if(index == NULL){
        ESP_LOGE(TAG, "Cannot open index");
        return NULL;
    }

    long offset = (long)(number - 1) * sizeof(UsbTrack);

    if(fseek(index, offset, SEEK_SET) != 0){
        ESP_LOGE(TAG, "Cannot seek position in index!");
        fclose(index);
        return NULL;
    }

    if(fread(&CurrentTrack, sizeof(UsbTrack), 1, index) != 1){
        ESP_LOGE(TAG, "Cannot read track by index!");
        fclose(index);
        return NULL;
    }

    fclose(index);

    return &CurrentTrack;
}
bool UsbLibrary_IsEmpty(void){
    return IsEmpty;
}
static uint32_t UsbLibrary_UpdateCrc32(uint32_t crc, const uint8_t *data, size_t length){
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
void UsbLibrary_SetSavedTrack(uint8_t track){
    SavedTrack = track;
}
uint8_t UsbLibrary_GetSavedTrack(void){
    return SavedTrack;
}
void UsbLibrary_SetSavedPage(uint8_t page){
    SavedPage = page;
}
uint8_t UsbLibrary_GetSavedPage(void){
    return SavedPage;
}
uint8_t UsbLibrary_GetVirtualTrack(uint16_t realTrack){
    return ((realTrack - 1) % TRACKS_PER_PAGE) + 1;
}
uint8_t UsbLibrary_GetVirtualPage(uint16_t realTrack){
    return (realTrack - 1) / TRACKS_PER_PAGE;
}
uint16_t UsbLibrary_GetRealTrackByPosition(uint8_t page, uint8_t track){
    return page * TRACKS_PER_PAGE + track;
}
uint32_t UsbLibrary_GetCurrentFingerprint(void){
    return CurrentCrc ^ 0xFFFFFFFF;
}
const UsbPlaylist *UsbLibrary_GetPlaylists(void){
    return PlayLists;
}
uint16_t UsbLibrary_GetPlaylistCount(void){
    return PlayListCount;
}
bool UsbLibrary_SetCurrentPlayList(uint8_t index){
    if(index < PlayListCount){
        CurrentPlaylist = &PlayLists[index];
        ESP_LOGI(TAG, "Selected Playlist %s", CurrentPlaylist->name);
        return true;
    } 
    CurrentPlaylist = NULL;
    return false;
}
void UsbLibrary_SetAllTracks(void){
    CurrentPlaylist = NULL;
    ESP_LOGI(TAG, "Selected full library");
}
static UsbFileType UsbLibrary_GetFileType(const char *path){
    const char *ext = strrchr(path, '.');
    if (ext == NULL)
        return OTHER;
    if(strcasecmp(ext, ".mp3") == 0)
        return MEDIA;
    else if(strcasecmp(ext, ".plst") == 0)
        return PLAYLIST;
    else return OTHER;
}
uint16_t UsbLibrary_GetCurrentPlaylistIndex(void){
    if (CurrentPlaylist == NULL)
        return 0;
    for (uint16_t i = 0; i < PlayListCount; i++){
        if (CurrentPlaylist == &PlayLists[i])
            return i + 1;
    }
    return 0;
}
UsbPlaylist *UsbLibrary_GetCurrentPlaylist(void){
    return CurrentPlaylist;
}
void UsbLibrary_SetCurrentPlaylistByName(const char *name){
    for (uint16_t i = 0; i < PlayListCount; i++){
        if (strcmp(name, PlayLists[i].name) == 0)
        {
            CurrentPlaylist = &PlayLists[i];
            ESP_LOGI(TAG, "Restored playlist: %s", CurrentPlaylist->name);
            return;
        }
    }
    ESP_LOGW(TAG, "Playlist not found: %s", name);
    CurrentPlaylist = NULL;
}