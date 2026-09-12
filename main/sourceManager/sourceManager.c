#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <esp_log.h>
#include <inttypes.h>
#include <string.h>

#include <sourceManager.h>
#include <usb/usbState.h>
#include <usb/usbStorage.h>
#include <cdc/cdc_state.h>
#include <bluetooth/bm83_state.h>
#include <usb/usbPlayer.h>
#include <bluetooth/bm83_player.h>
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
void SrcManager_SourceIn(void){
    ESP_LOGI(TAG, "Source IN");
    if(CdcState_GetCdcState() == CDC_NOCD){
        CdcState_CdcLoading();
        ProtocolDriveIn(); 
        if(BtState_GetLinkState() != LINK_DISCONNECTED){
            vTaskDelay(pdMS_TO_TICKS(500));
            CdcProtocol_CompleteLoad();
            CdcState_CdcStopPlay();
            CurrentPlayer = BT;
        }
    }
    ESP_LOGI(TAG, "Current player %s", CurrentPlayerToString());
}
void SrcManager_UsbReady(void){
    if(CdcState_GetCdcState() == CDC_LOADING){
        CdcProtocol_CompleteLoad();
        CdcState_CdcStopPlay();
    }
}
void SrcManager_SourceOut(void){
    ESP_LOGI(TAG, "Source OUT");
    UsbState usb = UsbState_GetState();
    BtLinkState link = BtState_GetLinkState();
    if(usb == USB_NODISK){
        if(link != LINK_DISCONNECTED)
        {
            CurrentPlayer = BT;
            BtPlayer_Play();
        }
        else
            CdcState_CdcNoDisk();
    }
    else
        UsbPlayer_Play();
    ESP_LOGI(TAG, "Current player %s", CurrentPlayerToString());
}
void SrcManager_ProcessEject(void){
    if(UsbState_GetState() != USB_NODISK){
        UsbPlayer_SaveCurrentTrackPage();
        UsbPlayer_Stop();
        CdcState_CdcStopPlay();    
        UsbStorageEject();
        UsbPlayer_Reset();
    }
    if(BtState_GetLinkState() != LINK_DISCONNECTED){
        BtPlayer_Stop();
    }
    CdcState_CdcEjectStart();
}
void SrcManager_CompleteEject(void){
    UsbState_Eject();
    CdcState_CdcNoDisk();
    if(BtState_GetLinkState() == LINK_STOP){
        vTaskDelay(pdMS_TO_TICKS(1000));
        SrcManager_SourceIn();
    }
    else
        BtState_SetLinkDisconnected();
}
void SrcManager_SendCurrentStatus(void){
    if(CurrentPlayer == USB)
        UsbPlayer_SendCurrentStatus();
    else
        BtPlayer_SendCurrentStatus();
}
#pragma endregion
void SrcManager_Play(void){
    ESP_LOGI(TAG, "PLAY called");
    UsbState usb = UsbState_GetState();
    BtLinkState link = BtState_GetLinkState();
    if(CurrentPlayer == USB){
        if(usb == USB_STOP || usb == USB_PLAY)
            UsbPlayer_Play();
        return;
    }
    if(CurrentPlayer == BT){
        if(link == LINK_STOP)
            BtPlayer_Play();
        return;
    }
}
void SrcManager_Stop(void){
    ESP_LOGI(TAG, "STOP called");
    if(CurrentPlayer == USB){
        if(CdcState_GetCdcState() == CDC_PLAY){
            UsbPlayer_Stop();
            CdcState_CdcStopPlay();
        }
        return;
    }
    if(CurrentPlayer == BT){
        if(BtState_GetLinkState() == LINK_PLAY || BtState_GetLinkState() == LINK_CALL){
            BtPlayer_Pause();
            CdcState_CdcStopPlay();
        }
        return;
    }
}
void SrcManager_SwitchTrack(uint8_t track){
     ESP_LOGI(TAG, "Switch Track");
    if(CurrentPlayer == USB){
        UsbPlayer_SwitchTrack(track);
        return;
    }
    if(CurrentPlayer == BT){
        BtPlayer_SwitchTrack(track);
        return;
    }
}