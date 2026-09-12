
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

CdcState CdcState_GetCdcState(void);
void CdcState_CdcBoot(void);
void CdcState_CdcStandby(void);
void CdcState_CdcLoading(void);
void CdcState_CdcPlay(void);
void CdcState_CdcStopPlay(void);
void CdcState_CdcEjectStart(void);
void CdcState_CdcNoDisk(void);