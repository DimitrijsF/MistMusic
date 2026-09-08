#pragma once

#include <stdbool.h>

typedef enum{
    STARTING,
    POWEROFF,
    POWERON,
    PAIRING
} BtState;

typedef enum{
    DISCONNECTED,
    CONNECTED,
    STOPPED,
    PLAYING,
    PAUSE,
    CALL
} BtLinkState;

BtLinkState BmState_GetLinkState(void);
BtState BmState_GetBtState(void);
bool BtState_IsInitialized(void);
void BtState_SetInitDone(void);
void BtState_Enable(void);
void BtState_SetOn(void);
void BtState_SetLinkConnected(void);