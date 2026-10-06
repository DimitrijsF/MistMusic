#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <esp_log.h>
#include <inttypes.h>
#include <string.h>

#include <playerManager.h>
#include <nvsManager.h>

#include <usb/usbState.h>
#include <usb/usbStorage.h>
#include <usb/usbLibrary.h>
#include <usb/usbPlayer.h>
#include <usb/usbHost.h>

#include <cdc/cdc_protocol.h>
#include <cdc/cdc_state.h>
#include <cdc/cdc_uart.h>

#include <wifi/wifiService.h>

#include <media/mediaOutput.h>

static const char *TAG = "PLAYER_MANAGER";

#pragma region Sources
void PlayerManager_SourceIn(void){
    ESP_LOGI(TAG, "Source IN");
    if(CdcState_GetCdcState() == CDC_NOCD){
        CdcState_CdcLoading();
        CdcProtocol_DriveIn(); 
    }
}
void PlayerManager_UsbReady(void){
    if(CdcState_GetCdcState() == CDC_LOADING){
        CdcProtocol_CompleteLoad();
        CdcState_CdcStopPlay();
    }
    if(UsbLibrary_GetCount() > 0){
        ESP_LOGI(TAG, "Usb ready");
        if(CdcState_GetCdcState() == CDC_PLAY){
            UsbPlayer_Play();
        }
    }
}
void PlayerManager_SourceOut(void){
    ESP_LOGI(TAG, "Source OUT");
    if(UsbState_GetState() == USB_NODISK){
        CdcState_CdcNoDisk();
    }
    else
    {
        if(CdcState_GetCdcState() == CDC_PLAY)
            UsbPlayer_Play();
        else 
            UsbPlayer_Stop();
    }
}
void PlayerManager_ProcessEject(void){
    ESP_LOGI(TAG, "EJECT called");
    CdcState_CdcEjectStart();
    if(UsbState_GetState() != USB_NODISK){
        UsbPlayer_SaveCurrentTrackPage();
        UsbPlayer_Stop(); 
        UsbStorageEject();
        UsbPlayer_Reset();
    }
}
void PlayerManager_CompleteEject(void){
    UsbState_Eject();
    CdcState_CdcNoDisk();
    NvsManager_SetDiskIn(false);
    NvsManager_Commit();
}
void PlayerManager_SendCurrentStatus(void){
    UsbPlayer_SendCurrentStatus();
}
#pragma endregion
#pragma region Arch methods
/// @brief Main system entry point (all systems init)
void PlayerManager_Init(void){
    NvsManager_Init();
    UsbState_SetUsbRandom(NvsManager_GetRandom());
    CdcUart_Init();
    UsbHost_Init();
    UsbStorage_Init();
    Output_Init();
    WifiService_Init();
    WifiService_Start();
}
#pragma endregion
void PlayerManager_Play(void){
    ESP_LOGI(TAG, "PLAY called");
    UsbState usb = UsbState_GetState();
    if(usb == USB_STOP || usb == USB_PLAY)
    {
        UsbPlayer_Play();
        CdcState_CdcPlay();
        UsbState_Play();
    }
}
void PlayerManager_Stop(void){
    ESP_LOGI(TAG, "STOP called");
    if(CdcState_GetCdcState() == CDC_PLAY){
        UsbPlayer_Stop();         
    }
    CdcState_CdcStopPlay();
}
void PlayerManager_SwitchTrack(uint8_t track){
    ESP_LOGI(TAG, "Switch Track");
    UsbPlayer_SwitchTrack(track);
    return;
}