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

void PlayerManager_SourceIn(void);
void PlayerManager_UsbReady(void);
void PlayerManager_SourceOut(void);
void PlayerManager_ProcessEject(void);
void PlayerManager_CompleteEject(void);
void PlayerManager_SendCurrentStatus(void);

void PlayerManager_Play(void);
void PlayerManager_Stop(void);
void PlayerManager_SwitchTrack(uint8_t track);