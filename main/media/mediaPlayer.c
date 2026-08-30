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

#define SEEK_UPDATE_INTERVAL_MS 200
#define SEEK_SLOW_BYTES 32000
#define SEEK_FAST_BYTES 160000
#define SEEK_SLOW_SECONDS 1
#define SEEK_FAST_SECONDS 5

static bool StateSaved = false;
static uint8_t CurrentPage = 0;
static uint8_t CurrentTrack = 1;
static uint32_t PlayedSeconds = 0;
static uint32_t PlayedSamples = 0;

static volatile bool playerStopRequested = false;
static volatile bool playerTrackChangeRequested = false;
static volatile uint8_t requestedPage = 0;
static volatile uint8_t requestedTrack = 0;
static volatile uint8_t requestedHeadTrack = 0;
static long ResumePosition = -1;
static uint16_t ResumeTrack = 0;

static volatile PlayState playerState = NORMAL;
static volatile bool fastSeek = false;
static volatile bool seekApplyRequested = false;

static long SeekPosition = 0;
static long SeekFileSize = 0;

static int64_t lastSeekUpdate = 0;

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
        uint16_t nextPageFirstTrack = (CurrentPage + 1) * TRACKS_PER_PAGE + 1;
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
        Player_CalcTrackSwitch();
    else
    {
        uint16_t requestedRealTrack = Player_GetRealTrackByPosition(CurrentPage, track);
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
static void Player_SendSeekStatus(void)
{
    uint32_t minutes = PlayedSeconds / 60;
    uint32_t seconds = PlayedSeconds % 60;
    PlayStatus status =
    {
        .Minutes = minutes,
        .Seconds = seconds,
        .Track = CurrentTrack
    };
    CdcProtocol_SendPlayStatus(status);
}
static void Player_ProcessSeek(void)
{
    int64_t now = esp_timer_get_time() / 1000;

    if(now - lastSeekUpdate <  SEEK_UPDATE_INTERVAL_MS)
    {
        vTaskDelay(pdMS_TO_TICKS(10));
        return;
    }

    lastSeekUpdate = now;

    long bytes =
        fastSeek
            ? SEEK_FAST_BYTES
            : SEEK_SLOW_BYTES;

    uint32_t secondsStep =
        fastSeek
            ? SEEK_FAST_SECONDS
            : SEEK_SLOW_SECONDS;

    if(playerState == FF)
    {
        SeekPosition += bytes;
        PlayedSeconds += secondsStep;
        if(SeekPosition >= SeekFileSize)
        {
            uint8_t nextPage;
            uint8_t nextTrack;
            uint8_t headTrack;

            Player_CalcNextTrack(
                &nextPage,
                &nextTrack,
                &headTrack);

            CurrentPage = nextPage;
            CurrentTrack = nextTrack;

            PlayedSeconds = 0;
            PlayedSamples = 0;

            SeekPosition = 0;

            SeekFileSize = Decoder_GetTrackSize(Player_GetRealTrack());

            if(SeekFileSize <= 0)
            {
                ESP_LOGE(
                    TAG,
                    "Cannot get next track size");

                playerState = PLAY;
                seekApplyRequested = true;

                return;
            }
            CdcProtocol_SendPlayStartPacket(CurrentTrack);
        }
    }
    else if(playerState == REW)
    {
        if(SeekPosition > bytes)
        {
            SeekPosition -= bytes;
            if(PlayedSeconds > secondsStep)
                PlayedSeconds -= secondsStep;
            else
                PlayedSeconds = 0;
        }
        else
        {
            uint16_t realTrack = Player_GetRealTrack();

            if(realTrack <= 1)
            {
                SeekPosition = 0;
                PlayedSeconds = 0;
            }
            else
            {
                realTrack--;
                CurrentPage = (realTrack - 1) / TRACKS_PER_PAGE;
                CurrentTrack = ((realTrack - 1) % TRACKS_PER_PAGE) + 1;
                SeekFileSize = Decoder_GetTrackSize( realTrack);
                if(SeekFileSize <= 0)
                {
                    SeekPosition = 0;
                    PlayedSeconds = 0;
                }
                else
                {
                    SeekPosition = SeekFileSize;
                    PlayedSeconds = 0;
                    PlayedSamples = 0;
                }

                CdcProtocol_SendPlayStartPacket( CurrentTrack);
            }
        }
    }
    Player_SendSeekStatus();
}
void Player_Play(void)
{
    if(playerTaskHandle != NULL)
    {
        if(playerState != NORMAL)
        {
            seekApplyRequested = true;
            playerState = NORMAL;
        }

        return;
    }

    playerStopRequested = false;
    playerTrackChangeRequested = false;
    playerState = NORMAL;

    BaseType_t result =
        xTaskCreate(
            PlayerTask,
            "PlayerTask",
            4096,
            NULL,
            5,
            &playerTaskHandle);

    if(result != pdPASS)
    {
        ESP_LOGE(TAG, "Failed to create player task");
        playerTaskHandle = NULL;
    }
}
static void PlayerTask(void *arg)
{
    uint16_t realTrack = Player_GetRealTrack();
    bool opened = false;

    if(ResumePosition >= 0 &&
       ResumeTrack == realTrack)
    {
        opened =
            Decoder_OpenAt(
                realTrack,
                ResumePosition);
        ResumePosition = -1;
        ResumeTrack = 0;
    }
    if(!opened)
        opened = Decoder_Open(realTrack);

    if(!opened)
        goto error_exit;

    SeekPosition = Decoder_GetPosition();
    SeekFileSize = Decoder_GetTrackSize(Player_GetRealTrack());

    CdcProtocol_SendPlayStartPacket(CurrentTrack);
    CdcPlay();

    while(GetCdcState() == PLAY)
    {
        if(playerStopRequested)
        {
            playerStopRequested = false;
            goto normal_exit;
        }

        if(playerTrackChangeRequested)
        {
            playerTrackChangeRequested = false;

            playerState = NORMAL;
            seekApplyRequested = false;

            Decoder_Close();

            CdcProtocol_SendStatusTocReady();
            CdcProtocol_SendPlayReadyPacket(requestedHeadTrack);

            CurrentPage = requestedPage;
            CurrentTrack = requestedTrack;

            CdcProtocol_SendPlayReadyPacket(CurrentTrack);

            CdcProtocol_SendStatusPlayReady();

            PlayedSeconds = 0;
            PlayedSamples = 0;

            if(!Decoder_Open(Player_GetRealTrack()))
                goto error_exit;

            if(requestedHeadTrack != CurrentTrack)
                CdcProtocol_SendPlayStartPacket( CurrentTrack);
            CdcPlay();
            continue;
        }

        if(seekApplyRequested)
        {
            seekApplyRequested = false;
            Decoder_Close();
            if(!Decoder_OpenAt( Player_GetRealTrack(), SeekPosition))
                goto error_exit;
            PlayedSamples = 0;
            playerState = PLAY;
            CdcPlay();
            continue;
        }
        if(playerState == FF || playerState == REW)
        {
            Player_ProcessSeek();
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

                if(headTrack ==
                SWITCH_TRACK_NUMBER)
                {
                    CdcProtocol_SendPlayStartPacket(
                        headTrack);

                    vTaskDelay(
                        pdMS_TO_TICKS(100));
                }

                CurrentPage = nextPage;
                CurrentTrack = nextTrack;

                PlayedSeconds = 0;
                PlayedSamples = 0;

                if(!Decoder_Open(
                    Player_GetRealTrack()))
                {
                    goto error_exit;
                }

                CdcProtocol_SendPlayStartPacket(
                    CurrentTrack);

                CdcPlay();

                break;
            }

            case DECODER_ERROR:
            {
                ESP_LOGE(
                    TAG,
                    "Playback error");

                uint8_t nextPage;
                uint8_t nextTrack;
                uint8_t headTrack;

                Decoder_Close();

                Player_CalcNextTrack(
                    &nextPage,
                    &nextTrack,
                    &headTrack);

                if(headTrack ==
                SWITCH_TRACK_NUMBER)
                {
                    CdcProtocol_SendPlayStartPacket(
                        headTrack);

                    vTaskDelay(
                        pdMS_TO_TICKS(100));
                }

                CurrentPage = nextPage;
                CurrentTrack = nextTrack;

                PlayedSeconds = 0;
                PlayedSamples = 0;

                if(!Decoder_Open(
                    Player_GetRealTrack()))
                {
                    goto error_exit;
                }

                CdcProtocol_SendPlayStartPacket(
                    CurrentTrack);

                CdcPlay();

                continue;
            }
        }
    }

normal_exit:
    ResumePosition = Decoder_GetPosition();
    ResumeTrack = Player_GetRealTrack();
    goto common_exit;
error_exit:
    ResumePosition = -1;
    ResumeTrack = 0;
    PlayedSeconds = 0;
    PlayedSamples = 0;
    goto common_exit;
common_exit:
    CdcStopPlay();
    Decoder_Close();
    Output_Stop();
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
void Player_FF(bool fast)
{
    if(playerTaskHandle == NULL)
        return;

    if(playerState == NORMAL)
    {
        SeekPosition =
            Decoder_GetPosition();

        SeekFileSize = Decoder_GetTrackSize(Player_GetRealTrack());

        if(SeekPosition < 0 || SeekFileSize <= 0)
            return;

        Output_Stop();
        playerState = FF;
        lastSeekUpdate =
            esp_timer_get_time() / 1000;
    }

    fastSeek = fast;
}

void Player_Rew(bool fast)
{
    if(playerTaskHandle == NULL)
        return;

    if(playerState == NORMAL)
    {
        SeekPosition = Decoder_GetPosition();
        SeekFileSize = Decoder_GetTrackSize( Player_GetRealTrack());
        if(SeekPosition < 0 || SeekFileSize <= 0)
            return;
        Output_Stop();
        playerState = REW;
        lastSeekUpdate = esp_timer_get_time() / 1000;
    }

    fastSeek = fast;
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
void Player_SetCurrentTrackPage(uint8_t track, uint8_t page){
    CurrentPage = page;
    CurrentTrack = track;
}
void Player_SaveCurrentTrackPage(void){
    if(!StateSaved)
    {
        StateSaved = true;
        MediaLibrary_SetSavedPage(CurrentPage);
        MediaLibrary_SetSavedTrack(CurrentTrack);
    }    
}
void Player_ResetSavedState(void){
    StateSaved = false;
}