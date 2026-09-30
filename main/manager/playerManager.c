#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <esp_log.h>
#include <inttypes.h>
#include <string.h>

#include <playerManager.h>
#include <usb/usbState.h>
#include <usb/usbStorage.h>
#include <usb/usbLibrary.h>
#include <cdc/cdc_state.h>
#include <usb/usbPlayer.h>
#include <cdc/cdc_protocol.h>

static const char *TAG = "SRC_MANAGER";

static ActivePlayer CurrentPlayer = USB;

ActivePlayer SrcManager_GetCurrentPlayer(void){
    return CurrentPlayer;
}
static const char *CurrentPlayerToString()
{
    switch (CurrentPlayer)
    {
        case USB: return "USB";
        case BT: return "BT";
        default: return "UNKNOWN";
    }
}
#pragma region Source change
void PlayerManager_SourceIn(void){
    ESP_LOGI(TAG, "Source IN");
    if(CdcState_GetCdcState() == CDC_NOCD){
        CdcState_CdcLoading();
        CdcProtocol_DriveIn(); 
    }
}
void PlayerManager_UsbReady(void){
    ESP_LOGI(TAG, "cdc state on usb ready %d", CdcState_GetCdcState());
    if(CdcState_GetCdcState() == CDC_LOADING){
        CdcProtocol_CompleteLoad();
        CdcState_CdcStopPlay();
    }
    if(UsbLibrary_GetCount() > 0){
        ESP_LOGI(TAG, "Usb ready");
        ESP_LOGI(TAG, "cdc state %d", CdcState_GetCdcState());
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
    ESP_LOGI(TAG, "Current player %s", CurrentPlayerToString());
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
}
void PlayerManager_SendCurrentStatus(void){
    UsbPlayer_SendCurrentStatus();
}
#pragma endregion
void SrcManager_Play(void){
    ESP_LOGI(TAG, "PLAY called");
    UsbState usb = UsbState_GetState();
    if(usb == USB_STOP || usb == USB_PLAY)
        UsbPlayer_Play();
    CdcState_CdcPlay();
}
void SrcManager_Stop(void){
    ESP_LOGI(TAG, "STOP called");
    if(CurrentPlayer == USB){
        if(CdcState_GetCdcState() == CDC_PLAY){
            UsbPlayer_Stop();         
        }
    }
    CdcState_CdcStopPlay();
}
void SrcManager_SwitchTrack(uint8_t track){
    ESP_LOGI(TAG, "Switch Track");
    if(CurrentPlayer == USB){
        UsbPlayer_SwitchTrack(track);
        return;
    }
}