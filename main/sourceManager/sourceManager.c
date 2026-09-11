#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <esp_log.h>
#include <inttypes.h>
#include <string.h>

#include <sourceManager.h>
#include <usb/usbState.h>
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
    if(GetCdcState() == CDC_NOCD){
        CdcLoadDisk();
        ProtocolDriveIn(); 
        if(BtState_GetLinkState() != LINK_DISCONNECTED){
            vTaskDelay(pdMS_TO_TICKS(500));
            CdcProtocol_CompleteLoad();
            CdcStopPlay();
        }
    }
    ESP_LOGI(TAG, "Current player %s", CurrentPlayerToString());
}
void SrcManager_UsbReady(void){
    if(GetCdcState() == CDC_LOADING){
        CdcProtocol_CompleteLoad();
        CdcStopPlay();
    }
}
void SrcManager_SourceOut(void){
    ESP_LOGI(TAG, "Source OUT");
    UsbState usb = UsbState_GetState();
    BtLinkState link = BtState_GetLinkState();
    if(usb == USB_NODISK){
        if(link != LINK_DISCONNECTED)
            BtPlayer_Play();
        else{
            CdcNoDisk();
        }
    }
    else
        UsbPlayer_Play();
    ESP_LOGI(TAG, "Current player %s", CurrentPlayerToString());
}
#pragma endregion
void SrcManager_Play(void){
    ESP_LOGI(TAG, "PLAY called");
    if(CurrentPlayer == USB){
        if(UsbState_GetState() != USB_PLAY)
            UsbPlayer_Play();
        return;
    }
    if(CurrentPlayer == BT){
        if(BtState_GetLinkState() != LINK_PLAY)
            BtPlayer_Play();
        return;
    }
}
void SrcManager_Stop(void){
    ESP_LOGI(TAG, "STOP called");
    if(CurrentPlayer == USB){
        if(GetCdcState() == CDC_PLAY){
            UsbPlayer_Stop();
            CdcStopPlay();
        }
        return;
    }
    if(CurrentPlayer == BT){
        if(BtState_GetLinkState() == LINK_PLAY || BtState_GetLinkState() == LINK_CALL){
            BtPlayer_Pause();
            CdcStopPlay();
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