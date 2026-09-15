#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <esp_log.h>

#include <bm83_player.h>
#include <bm83_state.h>
#include <sourceManager/sourceManager.h>
#include <cdc/cdc_protocol.h>
#include <bm83_protocol.h>

#include <media/mediaInput.h>
#include <media/mediaOutput.h>

#define MAX_PLAY_SECONDS 60 * 10
#define BT_CONSTANT_TRACK 10
#define AUDIO_BUFFER_SAMPLES 1024

static int16_t AudioBuffer[AUDIO_BUFFER_SAMPLES];

static const char* TAG = "BT_PLAYER";

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
        PlayedSeconds = 0;
        BtState_SetLinkStop();
        if (timerTaskHandle != NULL){
            vTaskDelete(timerTaskHandle);
            timerTaskHandle = NULL;
        }
        if(playerTaskHandle != NULL){
            vTaskDelete(playerTaskHandle);
            playerTaskHandle = NULL;
        }
        BtProto_SendStop();
    }
    Input_Stop();
    Output_Stop();
    IsPlaying = false;
}
void BtPlayer_Pause(void){
    BtState_SetLinkPause();
    BtProto_SendPause();
    BtPlayer_Stop();
}
void BtPlayer_SwitchTrack(uint8_t track){
    if(track > BT_CONSTANT_TRACK){
        BtProto_SendNextTrack();
    }
    else{
        BtProto_SendPrevTrack();
    }
    PlayedSeconds = 0;
}
static void PlayerTask(void *arg){
    CdcProtocol_SendPlayStartPacket(BT_CONSTANT_TRACK);
    BtState_SetLinkPlay();
    IsPlaying = true;
    if(timerTaskHandle == NULL)
        xTaskCreate(PlayerTimerTask, "PlayerTimer", 4096, NULL, 5, &timerTaskHandle);
    BtProto_SendPlay();
    while (BtState_GetLinkState() == LINK_PLAY)
    {
        if(!Input_IsStarted()){
            if(!Input_Start()){
                BtState_SetLinkStop();
                break;
            }
        }         
        if(!Output_IsStarted()){
            if(!Output_Start()){
                BtState_SetLinkStop();
                break;
            }
        }
        size_t samplesRead = Input_Read(AudioBuffer, AUDIO_BUFFER_SAMPLES);

        if(samplesRead == 0)
            continue;

        Output_Write(AudioBuffer, samplesRead);
    }
    BtPlayer_Stop();
}
static void PlayerTimerTask(void *arg){
    while(true){
        vTaskDelay(pdMS_TO_TICKS(1000));
        if(IsPlaying){
            PlayedSeconds++;
            if(PlayedSeconds > MAX_PLAY_SECONDS)
                PlayedSeconds = 0;       
            BtPlayer_SendCurrentStatus();  
        }
    }
}
void BtPlayer_SendCurrentStatus(void){
    PlayStatus status =
    {
        .Minutes = PlayedSeconds / 60,
        .Seconds = PlayedSeconds % 60,
        .Track = BT_CONSTANT_TRACK
    };
    CdcProtocol_SendPlayStatus(status);
}