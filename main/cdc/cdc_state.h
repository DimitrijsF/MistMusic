
#pragma once

typedef enum
{
    CDC_STANDBY,
    CDC_BOOT,
    CDC_NOCD,
    CDC_LOADING,
    CDC_EJECTING,
    CDC_PLAY,
    CDC_STOP
} CdcState;

CdcState GetCdcState(void);
void CdcBoot(void);
void CdcStandby(void);
void CdcLoadDisk(void);
void CdcPlay(void);
void CdcStopPlay(void);
void CdcEjectStart(void);
void CdcNoDisk(void);