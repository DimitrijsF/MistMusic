#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <esp_log.h>

#include <bm83_player.h>
#include <bm83_state.h>
#include <sourceManager/sourceManager.h>
#include <cdc/cdc_protocol.h>

#define MAX_PLAY_SECONDS 60 * 10

static const char* TAG = "BT_PLAYER";

static uint8_t CurrentTrack = 10;
static uint16_t PlayedSeconds = 0;
static bool IsPlaying = false;

static void PlayerTimerTask(void *arg);
static void PlayerTask(void *arg);
static TaskHandle_t timerTaskHandle = NULL;
static TaskHandle_t playerTaskHandle = NULL;

void BtPlayer_Play(void){
    if(playerTaskHandle != NULL){
        return;
    }
    xTaskCreate(PlayerTask, "PlayerTask", 4096, NULL, 5, &playerTaskHandle);
    ESP_LOGI(TAG, "BT Player started");
}
void BtPlayer_Stop(void){
    if(BtState_GetLinkState() != LINK_PAUSE){
        BtState_SetLinkStop();
        if (timerTaskHandle != NULL)
        {
            vTaskDelete(timerTaskHandle);
            timerTaskHandle = NULL;
        }
        IsPlaying = false;
        //send bt stop
    }
}
void BtPlayer_Pause(void){
    BtState_SetLinkPause();
    IsPlaying = false;
    BtPlayer_Stop();
    //send bt pause
}
void BtPlayer_SwitchTrack(uint8_t track){
    if(track > CurrentTrack){
        //send bt next
    }
    else{
        //send bt prev
    }
    PlayedSeconds = 0;
}
static void PlayerTask(void *arg){
    CdcProtocol_SendPlayStartPacket(CurrentTrack);
    BtState_SetLinkPlay();
    IsPlaying = true;
    if(timerTaskHandle == NULL)
        xTaskCreate(PlayerTimerTask, "PlayerTimer", 4096, NULL, 5, &timerTaskHandle);
    while (BtState_GetLinkState() == LINK_PLAY)
    {
        //playing bt stream
    }
}
static void PlayerTimerTask(void *arg){
    while(true){
        vTaskDelay(pdMS_TO_TICKS(1000));
        if(IsPlaying){
            PlayedSeconds++;
            if(PlayedSeconds > MAX_PLAY_SECONDS)
                PlayedSeconds = 0;
            PlayStatus status =
            {
                .Minutes = PlayedSeconds / 60,
                .Seconds = PlayedSeconds % 60,
                .Track = CurrentTrack
            };
            CdcProtocol_SendPlayStatus(status);
        }
    }
}