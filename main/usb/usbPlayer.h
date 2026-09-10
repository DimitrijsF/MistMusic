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

void UsbPlayer_SwitchTrack(uint8_t track);
void UsbPlayer_Play(void);
void UsbPlayer_Stop(void);
void UsbPlayer_Reset(void);
void UsbPlayer_UpdateTime(uint16_t samples, uint32_t sampleRate);
void UsbPlayer_SetCurrentTrackPage(uint8_t track, uint8_t page);
void UsbPlayer_SaveCurrentTrackPage(void);
void UsbPlayer_ResetSavedState(void);
void UsbPlayer_SendCurrentStatus(void);