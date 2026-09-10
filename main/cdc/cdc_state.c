#include <stdbool.h>
#include <string.h>

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <cdc_state.h>
#include <cdc_protocol.h>
#include <cdc_uart.h>
#include <usb/usbLibrary.h>
#include <usb/usbPlayer.h>
#include <usb/usbStorage.h>

static bool UsbRandom = false;

static CdcState State = CDC_STANDBY;
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
        case CDC_STANDBY: return "CDC_STANDBY";
        case CDC_BOOT:    return "CDC_BOOT";
        case CDC_NOCD: return "CDC_NOCD";
        case CDC_LOADING: return "CDC_LOADING";
        case CDC_EJECTING: return "CDC_EJECTING";
        case CDC_PLAY: return "CDC_PLAY";
        case CDC_STOP: return "CDC_STOP";
        default: return "UNKNOWN";
    }
}
void CdcBoot(void){
    SetCdcState(CDC_BOOT);
    CdcUart_Init();
    vTaskDelay(pdMS_TO_TICKS(500));
    if(UsbStorage_DriveIn() && !UsbLibrary_IsEmpty())
        SetCdcState(CDC_STOP); 
    else
        SetCdcState(CDC_NOCD);
}
void CdcStandby(void){
    SetCdcState(CDC_STANDBY);
    UartShutDown();
}
void CdcLoadDisk(void){
    SetCdcState(CDC_LOADING);
    ProtocolDriveIn();
}
void CdcPlay(void){
    SetCdcState(CDC_PLAY);
}
void CdcStopPlay(void){
    if(State != CDC_NOCD)
        SetCdcState(CDC_STOP);
    SetCdcState(CDC_STOP);
}
void CdcEjectStart(void){
    SetCdcState(CDC_EJECTING);
}
void CdcNoDisk(void){
    SetCdcState(CDC_NOCD);
}
void CdcSetUsbRandom(bool value){
    UsbRandom = value;
}
bool CdcGetUsbRandom(void){
    return UsbRandom;
}