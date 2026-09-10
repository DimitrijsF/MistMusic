#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <esp_log.h>
#include <inttypes.h>

#include <sourceManager.h>
#include <cdc/cdc_state.h>
#include <bluetooth/bm83_state.h>

static const char *TAG = "SRC_MANAGER";

static ActivePlayer CurrentPlayer = USB;

ActivePlayer SrcManager_GetCurrentPlayer(void){
    return CurrentPlayer;
}
void SrcManager_CheckSource(void){
    ActivePlayer current = CurrentPlayer;
    CdcState cdcState = GetCdcState();
    BtLinkState linkState = BmState_GetLinkState();
    if(cdcState == NO_DISK){
        if(linkState == CONNECTED)
            CurrentPlayer = BT;
        else
            CurrentPlayer = USB;
    }
    else
        CurrentPlayer = USB;
    if(current != CurrentPlayer)
        ESP_LOGI(TAG, "Source changed %s -> %s", current, CurrentPlayer);
}
void SrcManager_Play(void){
    if(CurrentPlayer == USB){
        if(GetCdcState() != PLAY)
            UsbPlayer_Play();
        return;
    }
    if(CurrentPlayer == BT){
        return;
    }
}
void SrcManager_Stop(void){
    if(CurrentPlayer == USB){
        if(GetCdcState() == PLAY){
            UsbPlayer_Stop();
            CdcStopPlay();
        }
        return;
    }
    if(CurrentPlayer == BT){
        return;
    }
}
void SrcManager_SwitchTrack(uint8_t track){
    if(CurrentPlayer == USB){
        UsbPlayer_SwitchTrack(track);
        return;
    }
    if(CurrentPlayer == BT){
        return;
    }
}