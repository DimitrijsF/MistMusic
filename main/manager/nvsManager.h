#pragma once
#include <stdbool.h>
#include <inttypes.h>

bool NvsManager_Init(void);

uint16_t NvsManager_GetTrack(void);
bool NvsManager_SetTrack(uint16_t track);

bool NvsManager_GetRandom(void);
bool NvsManager_SetRandom(bool random);

bool NvsManager_Commit(void);