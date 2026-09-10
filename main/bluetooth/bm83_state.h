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
    LINK_CONNECTED,
    LINK_STOP,
    LINK_PLAY,
    LINK_PAUSE,
    LINK_CALL
} BtLinkState;

BtLinkState BmState_GetLinkState(void);
BtState BmState_GetBtState(void);
bool BtState_IsInitialized(void);
void BtState_SetInitDone(void);
void BtState_Enable(void);
void BtState_SetOn(void);
void BtState_EnablePairing(void);
void BtState_SetLinkConnected(void);
void BtState_SetLinkDisconnected(void);