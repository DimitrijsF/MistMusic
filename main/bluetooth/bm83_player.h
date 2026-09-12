#pragma once

#include <inttypes.h>

void BtPlayer_Play(void);
void BtPlayer_Stop(void);
void BtPlayer_Pause(void);
void BtPlayer_SwitchTrack(uint8_t track);
void BtPlayer_SendCurrentStatus(void);