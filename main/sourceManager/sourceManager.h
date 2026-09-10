#pragma once

typedef enum{
    USB,
    BT
} ActivePlayer;

ActivePlayer SrcManager_GetCurrentPlayer(void);
void SrcManager_CheckSource(void);
void SrcManager_Play(void);
void SrcManager_Stop(void);
void SrcManager_SwitchTrack(uint8_t track);