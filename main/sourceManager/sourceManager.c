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

static const char *TAG = "SRC_MANAGER";

static ActivePlayer CurrentPlayer = USB;

ActivePlayer SrcManager_GetCurrentPlayer(void){
    return CurrentPlayer;
}
static const char *DeviceStateToString(ActivePlayer player)
{
    switch (player)
    {
        case USB: return "USB";
        case BT: return "BT";
        default: return "UNKNOWN";
    }
}
void SrcManager_CheckSource(void){
    ActivePlayer current = CurrentPlayer;
    CdcState cdcState = GetCdcState();
    BtLinkState linkState = BmState_GetLinkState();
    if(cdcState == CDC_NOCD){
        if(linkState == LINK_CONNECTED)
            CurrentPlayer = BT;
        else
            CurrentPlayer = USB;
    }
    else
        CurrentPlayer = USB;
    if(current != CurrentPlayer)
        ESP_LOGI(TAG, "Source changed %s -> %s", DeviceStateToString(current), DeviceStateToString(CurrentPlayer));
}
void SrcManager_Play(void){
    if(CurrentPlayer == USB){
        if(GetCdcState() != CDC_PLAY)
            UsbPlayer_Play();
        return;
    }
    if(CurrentPlayer == BT){
        return;
    }
}
void SrcManager_Stop(void){
    if(CurrentPlayer == USB){
        if(GetCdcState() == CDC_PLAY){
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