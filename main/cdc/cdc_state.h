#pragma once

typedef enum
{
    STANDBY,
    BOOT,
    NO_DISK,
    LOADING,
    EJECTING,
    PLAY,
    STOP
} CdcState;

CdcState GetCdcState(void);
void CdcBoot(void);
void CdcStandby(void);
void CdcLoadDisk(void);
void CdcPlay(void);
void CdcStopPlay(void);
void CdcEjectStart(void);
void CdcNoDisk(void);