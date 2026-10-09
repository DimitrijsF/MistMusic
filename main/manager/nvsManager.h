#pragma once
#include <stdbool.h>
#include <inttypes.h>

bool NvsManager_Init(void);

uint16_t NvsManager_GetTrack(void);
bool NvsManager_SetTrack(uint16_t track);

bool NvsManager_GetRandom(void);
bool NvsManager_SetRandom(bool random);

bool NvsManager_GetDiskIn(void);
bool NvsManager_SetDiskIn(bool diskIn);

uint32_t NvsManager_GetFingerprint(void);
bool NvsManager_SetFingerprint(uint32_t fingerprint);

uint32_t NvsManager_GetResumeSeconds(void);
bool NvsManager_SetResumeSeconds(uint32_t seconds);

long NvsManager_GetResumePosition(void);
bool NvsManager_SetResumePosition(long position);

bool NvsManager_GetPlaylist(char *playlist, size_t size);
bool NvsManager_SetPlaylist(const char *name);

bool NvsManager_Commit(void);