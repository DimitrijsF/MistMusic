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
#include <media/mediaOutput.h>
#include <mediaDecoder.h>

#define TRACKS_PER_PAGE 127
#define SWITCH_TRACK_NUMBER 128

static uint8_t CurrentPage = 0;
static uint8_t CurrentTrack = 1;
static uint32_t PlayedSeconds = 0;
static uint32_t PlayedSamples = 0;

static volatile bool playerStopRequested = false;
static volatile bool playerTrackChangeRequested = false;
static volatile uint8_t requestedPage = 0;
static volatile uint8_t requestedTrack = 0;
static volatile uint8_t requestedHeadTrack = 0;

static const char* TAG = "MEDIA_PLAYER";

static void PlayerTask(void *arg);
static TaskHandle_t playerTaskHandle = NULL;

static uint16_t Player_GetRealTrackByPosition(
    uint8_t page,
    uint8_t track)
{
    return page * TRACKS_PER_PAGE + track;
}
static uint16_t Player_GetRealTrack()
{
    return CurrentPage * TRACKS_PER_PAGE + CurrentTrack;
}
static void Player_CalcTrackSwitch(void)
{
    uint16_t trackCount = MediaLibrary_GetCount();
    requestedPage = CurrentPage;
    if(CurrentTrack == TRACKS_PER_PAGE)
    {
        uint16_t nextPageFirstTrack =
            (CurrentPage + 1) * TRACKS_PER_PAGE + 1;
        if(nextPageFirstTrack <= trackCount)
        {
            requestedPage = CurrentPage + 1;
            requestedTrack = 1;
        }
        else
        {
            requestedPage = 0;
            requestedTrack = 1;
        }
        return;
    }
    if(CurrentTrack == 1)
    {
        if(CurrentPage == 0)
        {
            uint16_t lastRealTrack = trackCount - 1;
            requestedPage =
                lastRealTrack / TRACKS_PER_PAGE;
            requestedTrack =
                (lastRealTrack % TRACKS_PER_PAGE) + 1;
        }
        else
        {
            requestedPage = CurrentPage - 1;
            requestedTrack = TRACKS_PER_PAGE;
        }
        return;
    }
    requestedTrack = CurrentTrack;
}
void Player_SwitchTrack(uint8_t track)
{
    requestedHeadTrack = track;
    requestedPage = CurrentPage;

    if(track == SWITCH_TRACK_NUMBER)
    {
        Player_CalcTrackSwitch();
    }
    else
    {
        uint16_t requestedRealTrack =
            Player_GetRealTrackByPosition(
                CurrentPage,
                track);
        if(requestedRealTrack > MediaLibrary_GetCount())
        {
            requestedPage = 0;
            requestedTrack = 1;
        }
        else
            requestedTrack = track;
    }
    PlayedSeconds = 0;
    PlayedSamples = 0;
    playerTrackChangeRequested = true;
}
static bool Player_CalcNextTrack(
    uint8_t *nextPage,
    uint8_t *nextTrack,
    uint8_t *headTrack)
{
    uint16_t realTrack = Player_GetRealTrack();
    uint16_t trackCount = MediaLibrary_GetCount();

    *nextPage = CurrentPage;
    *nextTrack = CurrentTrack;
    *headTrack = 0;
    if(realTrack >= trackCount)
    {
        *nextPage = 0;
        *nextTrack = 1;
        *headTrack = 1;

        return true;
    }
    if(CurrentTrack >= TRACKS_PER_PAGE)
    {
        *nextPage = CurrentPage + 1;
        *nextTrack = 1;
        *headTrack = SWITCH_TRACK_NUMBER;

        return true;
    }
    *nextTrack = CurrentTrack + 1;
    *headTrack = *nextTrack;

    return true;
}

void Player_Play(void){
    playerStopRequested = false;
    playerTrackChangeRequested = false;
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
    if(!Decoder_Open(Player_GetRealTrack()))
        goto exit;

    CdcProtocol_SendPlayStartPacket(CurrentTrack);
    CdcPlay();

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
            CdcProtocol_SendStatusTocReady();
            CdcProtocol_SendPlayReadyPacket(requestedHeadTrack);
            vTaskDelay(pdMS_TO_TICKS(100));
            CdcProtocol_SendStatusPlayReady();
            CdcProtocol_SendPlayStartPacket(requestedHeadTrack);
            CurrentPage = requestedPage;
            CurrentTrack = requestedTrack;
            if(!Decoder_Open(Player_GetRealTrack()))
                goto exit;
            if(requestedHeadTrack != CurrentTrack)
                CdcProtocol_SendPlayStartPacket(CurrentTrack);
            CdcPlay();
            continue;
        }

        switch(MediaDecoder_Step())
        {
            case DECODER_OK:
                break;

            case DECODER_EOF:
            {
                uint8_t nextPage;
                uint8_t nextTrack;
                uint8_t headTrack;
                Decoder_Close();
                Player_CalcNextTrack(
                    &nextPage,
                    &nextTrack,
                    &headTrack);
                if(headTrack == SWITCH_TRACK_NUMBER)
                {
                    CdcProtocol_SendPlayStartPacket(
                        headTrack);

                    vTaskDelay(pdMS_TO_TICKS(100));
                }
                CurrentPage = nextPage;
                CurrentTrack = nextTrack;
                PlayedSeconds = 0;
                PlayedSamples = 0;
                if(!Decoder_Open(Player_GetRealTrack()))
                    goto exit;
                CdcProtocol_SendPlayStartPacket(
                    CurrentTrack);
                CdcPlay();
                break;
            }
            case DECODER_ERROR:
            {
                ESP_LOGE(TAG, "Playback error");
                uint8_t nextPage;
                uint8_t nextTrack;
                uint8_t headTrack;
                Decoder_Close();
                Player_CalcNextTrack(
                    &nextPage,
                    &nextTrack,
                    &headTrack);
                if(headTrack == SWITCH_TRACK_NUMBER)
                {
                    CdcProtocol_SendPlayStartPacket(
                        headTrack);
                    vTaskDelay(pdMS_TO_TICKS(100));
                }
                CurrentPage = nextPage;
                CurrentTrack = nextTrack;
                PlayedSeconds = 0;
                PlayedSamples = 0;
                if(!Decoder_Open(Player_GetRealTrack()))
                    goto exit;
                CdcProtocol_SendPlayStartPacket(
                    CurrentTrack);
                CdcPlay();
                continue;
            }
        }
    }

exit:
    CdcStopPlay();
    Decoder_Close();
    Output_Stop();
    PlayedSeconds = 0;
    PlayedSamples = 0;
    playerTaskHandle = NULL;
    playerStopRequested = false;
    playerTrackChangeRequested = false;
    vTaskDelete(NULL);
}

void Player_Stop(void){
    if(GetCdcState() != NO_DISK)
        playerStopRequested = true;
}
void Player_Reset(void){
    CurrentPage = 0;
    CurrentTrack = 1;
    PlayedSeconds = 0;
    PlayedSamples = 0;
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

    uint32_t Minutes = second / 60;
    uint32_t Seconds = second % 60;

    PlayStatus status =
    {
        .Minutes = Minutes,
        .Seconds = Seconds,
        .Track = CurrentTrack
    };
    
    CdcProtocol_SendPlayStatus(status);
}