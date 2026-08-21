#include <inttypes.h>
#include <stdbool.h>
#include "esp_log.h"
#include <stdio.h>
#include "esp_timer.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <cdc/cdc_state.h>

#include <cdc/cdc_protocol.h>

#include <media/mediaPlayer.h>
#include <media/mediaLibrary.h>
#include <mediaDecoder.h>

static uint8_t CurrentTrack = 1;
static uint8_t PlayedSeconds = 0;
static uint32_t PlayedSamples = 0;

static volatile bool playerStopRequested = false;
static volatile bool playerTrackChangeRequested = false;
static volatile uint8_t requestedTrack = 0;

static const char* TAG = "MEDIA_PLAYER";

static void PlayerTask(void *arg);
static TaskHandle_t playerTaskHandle = NULL;
static void SendPlayStart(void);

void Player_SwitchTrack(uint8_t track){
    if(track > MediaLibrary_GetCount())
        requestedTrack = 1;
    else
        requestedTrack = track;

    PlayedSeconds = 0;
    PlayedSamples = 0;

    playerTrackChangeRequested = true;
}

void Player_Play(void){
     xTaskCreate(
        PlayerTask,
        "CdcUart",
        4096,
        NULL,
        5,
        &playerTaskHandle);
}
static void PlayerTask(void *arg)
{
    if(!Decoder_Open(CurrentTrack))
        goto exit;

    CdcPlay();
    SendPlayStart();
    while(GetCdcState() == PLAY)
    {
        if(playerStopRequested)
        {
            playerStopRequested = false;
            goto exit;
        }

        if(playerTrackChangeRequested)
        {
            playerTrackChangeRequested = false;
            Decoder_Close();
            CurrentTrack = requestedTrack;
            if(!Decoder_Open(CurrentTrack))
                goto exit;
            CdcPlay();
            SendPlayStart();
            continue;
        }

        switch(MediaDecoder_Step())
        {
            case DECODER_OK:
                break;
            case DECODER_EOF:
                ESP_LOGI(TAG, "Switching track...");
                Decoder_Close();
                if(CurrentTrack >= MediaLibrary_GetCount())
                    CurrentTrack = 1;
                else
                    CurrentTrack++;
                PlayedSeconds = 0;
                PlayedSamples = 0;

                if(!Decoder_Open(CurrentTrack))
                    goto exit;
                CdcPlay();
                SendPlayStart();
                break;

            case DECODER_ERROR:
                ESP_LOGE(TAG, "Playback error");
                goto exit;
        }
    }

exit:
    ESP_LOGI(TAG, "stopping from exit");
    CdcStopPlay();
    Decoder_Close();
    playerTaskHandle = NULL;
    vTaskDelete(NULL);
}

static void SendPlayStart(void){
    PlayStatus status =
    {
        .Minutes = 0,
        .Seconds = 0,
        .Track = CurrentTrack
    };
    CdcProtocol_SendPlayStatus(status);
}

void Player_Stop(void){
    ESP_LOGI(TAG, "stopping from Player_Stop");
    if(GetCdcState() != NO_DISK)
        CdcStopPlay();
    playerStopRequested = true;

}

void Player_FF(bool enable){

}

void Player_Rew(bool enable){

}

void Player_UpdateTime(uint16_t samples, uint32_t sampleRate){
    if(sampleRate == 0)
        return;

    PlayedSamples += samples;
    uint32_t second = PlayedSamples / sampleRate;

    if(second == PlayedSeconds)
        return;

    PlayedSeconds = second;

    uint8_t Minutes = second / 60;
    uint8_t Seconds = second % 60;

    PlayStatus status =
    {
        .Minutes = Minutes,
        .Seconds = Seconds,
        .Track = CurrentTrack
    };
    CdcProtocol_SendPlayStatus(status);
}