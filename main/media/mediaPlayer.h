#pragma once

#include <inttypes.h>
#include <stdbool.h>

typedef struct
{
    uint8_t Minutes;
    uint8_t Seconds;
    uint8_t Track;
} PlayStatus;

void Player_SwitchTrack(uint8_t track);
void Player_Play(void);
void Player_Stop(void);
void Player_FF(bool enable);
void Player_Rew(bool enable);
void Player_UpdateTime(uint16_t samples, uint32_t sampleRate);