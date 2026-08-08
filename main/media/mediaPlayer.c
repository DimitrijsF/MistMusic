#include <inttypes.h>
#include <stdbool.h>
#include "esp_log.h"
#include <stdio.h>
#include "esp_timer.h"

#include <cdc/cdc_state.h>

#include <cdc/cdc_protocol.h>

#include <media/mediaPlayer.h>
#include <media/mediaLibrary.h>
#include <mediaDecoder.h>

static uint8_t CurrentTrack = 1;
static uint8_t PlayedSeconds = 0;
static uint32_t PlayedSamples = 0;

static const char* TAG = "MEDIA_PLAYER";

void Player_SwitchTrack(uint8_t track){
    CdcStopPlay();
    Decoder_Close();

    if(track > MediaLibrary_GetCount())
        CurrentTrack = 1;
    else
        CurrentTrack = track;
    PlayedSeconds = 0;
    PlayedSamples = 0;
    Player_Play();
}

void Player_Play(void){
    CdcPlay();
    
    if(!Decoder_Open(CurrentTrack))
        return;
    while(GetCdcState() == PLAY)
    {   
        switch(MediaDecoder_Step())
        {
            case DECODER_OK:
                break;

            case DECODER_EOF:      
                ESP_LOGI(TAG, "Switching track...");
                Player_SwitchTrack(CurrentTrack + 1);
                break;

            case DECODER_ERROR:
                Player_Stop();
                break;
        }
    }
}

void Player_Stop(void){
    CdcStopPlay();
    Decoder_Close();
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