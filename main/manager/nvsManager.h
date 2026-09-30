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

bool NvsManager_Commit(void);