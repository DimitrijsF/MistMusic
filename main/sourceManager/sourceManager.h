#pragma once

#include <inttypes.h>

typedef enum{
    USB,
    BT
} ActivePlayer;
typedef struct
{
    uint8_t Minutes;
    uint8_t Seconds;
    uint8_t Track;
} PlayStatus;

ActivePlayer SrcManager_GetCurrentPlayer(void);

void SrcManager_SourceIn(void);
void SrcManager_UsbReady(void);
void SrcManager_SourceOut(void);
void SrcManager_ProcessEject(void);
void SrcManager_CompleteEject(void);
void SrcManager_SendCurrentStatus(void);

void SrcManager_Play(void);
void SrcManager_Stop(void);
void SrcManager_SwitchTrack(uint8_t track);