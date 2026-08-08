
#pragma once

typedef enum
{
    STANDBY,
    BOOT,
    NO_DISK,
    LOADING,
    READY,
    EJECTING,
    PLAY,
    STOP
} CdcState;

CdcState GetCdcState(void);
void CdcBoot(void);
void CdcStandby(void);
void CdcReadDrive(void);
void CdcReadyToPlay(void);
void CdcPlay(void);
void CdcStopPlay(void);