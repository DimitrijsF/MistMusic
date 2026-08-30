#pragma once

#include <inttypes.h>
#include <stdbool.h>

#define TRACKS_PER_PAGE 99
#define SWITCH_TRACK_NUMBER 100

typedef struct
{
    uint8_t Minutes;
    uint8_t Seconds;
    uint8_t Track;
} PlayStatus;

typedef enum{
    NORMAL,
    FF,
    REW
} PlayState;

void Player_SwitchTrack(uint8_t track);
void Player_Play(void);
void Player_Stop(void);
void Player_Reset(void);
void Player_FF(bool enable);
void Player_Rew(bool enable);
void Player_UpdateTime(uint16_t samples, uint32_t sampleRate);
void Player_SetCurrentTrackPage(uint8_t track, uint8_t page);
void Player_SaveCurrentTrackPage(void);