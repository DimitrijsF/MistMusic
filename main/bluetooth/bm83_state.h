#pragma once

#include <stdbool.h>

typedef enum{
    BT_STARTING,
    BT_POWEROFF,
    BT_POWERON,
    BT_PAIRING
} BtState;

typedef enum{
    LINK_DISCONNECTED,
    LINK_STOP,
    LINK_PLAY,
    LINK_PAUSE,
    LINK_CALL
} BtLinkState;

BtLinkState BtState_GetLinkState(void);
BtState BtState_GetBtState(void);
bool BtState_IsInitialized(void);
void BtState_SetInitDone(void);
void BtState_Enable(void);
void BtState_SetOn(void);
void BtState_EnablePairing(void);

void BtState_SetLinkConnected(void);
void BtState_SetLinkDisconnected(void);
void BtState_SetLinkPlay(void);
void BtState_SetLinkStop(void);
void BtState_SetLinkPause(void);