#include <stdbool.h>
#include <string.h>

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <cdc_state.h>
#include <cdc_protocol.h>
#include <cdc_uart.h>
#include <media/mediaLibrary.h>
#include <media/mediaPlayer.h>
#include <usb/usbStorage.h>

static CdcState State = STANDBY;
static const char *TAG = "CDC_STATE";

static const char *StateToString(CdcState state);

CdcState GetCdcState(void){
    return State;
}
static void SetCdcState(CdcState state){
    if(State == state)
        return;
    ESP_LOGI(TAG,
         "%s -> %s",
         StateToString(State),
         StateToString(state));
    State = state;
}
static const char *StateToString(CdcState state)
{
    switch (state)
    {
        case STANDBY: return "STANDBY";
        case BOOT:    return "BOOT";
        case NO_DISK: return "NO_DISK";
        case LOADING: return "LOADING";
        case EJECTING: return "EJECTING";
        case PLAY: return "PLAY";
        case STOP: return "STOP";
        default: return "UNKNOWN";
    }
}
void CdcBoot(void){
    SetCdcState(BOOT);
    CdcUart_Init();
    vTaskDelay(pdMS_TO_TICKS(500));
    if(UsbStorage_DriveIn() && !MediaLibrary_IsEmpty())
        SetCdcState(STOP); 
    else
        SetCdcState(NO_DISK);
}
void CdcStandby(void){
    SetCdcState(STANDBY);
    UartShutDown();
}
void CdcLoadDisk(void){
    SetCdcState(LOADING);
    ProtocolDriveIn();
}
void CdcPlay(void){
    SetCdcState(PLAY);
}
void CdcStopPlay(void){
    if(State != NO_DISK)
        SetCdcState(STOP);
    SetCdcState(STOP);
}
void CdcEjectStart(void){
    SetCdcState(EJECTING);
}
void CdcNoDisk(void){
    SetCdcState(NO_DISK);
}