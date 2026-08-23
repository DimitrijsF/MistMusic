#include <inttypes.h>
#include <stdbool.h>
#include "esp_log.h"
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"

#include <media/mediaPlayer.h>
#include <media/mediaLibrary.h>
#include <media/mediaDecoder.h>
#include <media/mediaOutput.h>

#include <mp3dec.h>

#define MP3_INPUT_BUFFER_SIZE    4096
#define MP3_REFILL_THRESHOLD 2048
#define PCM_BUFFER_SAMPLES       (1152 * 2)

#pragma region Static Methods
static bool SkipMetadata(FILE *file);
static bool FillBuffer(void);
static DecoderResult Decoder_DecodeFrame(void);
#pragma endregion

static MP3FrameInfo g_FrameInfo = {0};

static HMP3Decoder g_Decoder = NULL;
static FILE *g_File = NULL;

static uint8_t g_InputBuffer[MP3_INPUT_BUFFER_SIZE];
static const unsigned char *g_ReadPtr = NULL;
static size_t g_BytesLeft = 0;

static short g_PcmBuffer[PCM_BUFFER_SAMPLES];

static const char *TAG = "MEDIA_DECODER";

static bool FillBuffer(void)
{
    if(g_File == NULL)
        return false;

    if(g_BytesLeft > 0)
    {
        memmove(
            g_InputBuffer,
            g_ReadPtr,
            g_BytesLeft);
    }

    size_t bytesRead =
        fread(
            g_InputBuffer + g_BytesLeft,
            1,
            sizeof(g_InputBuffer) - g_BytesLeft,
            g_File);

    g_ReadPtr = g_InputBuffer;
    g_BytesLeft += bytesRead;

    return (bytesRead > 0);
}

bool Decoder_Open(uint16_t trackNumber)
{
    MediaTrack *track = MediaLibrary_GetTrack(trackNumber);

    if(track == NULL)
        return false;

    ESP_LOGI(TAG, "Playing %s, Track number %u", track->Path, trackNumber);

    g_Decoder = MP3InitDecoder();

    if(g_Decoder == NULL)
        return false;

    g_File = fopen(track->Path, "rb");

    if(g_File == NULL)
    {
        MP3FreeDecoder(g_Decoder);
        g_Decoder = NULL;
        return false;
    }

    if(!SkipMetadata(g_File))
    {
        ESP_LOGI(TAG, "Metadata Error");
        Decoder_Close();
        return false;
    }

    size_t bytesRead =
        fread(
            g_InputBuffer,
            1,
            sizeof(g_InputBuffer),
            g_File);

    if(bytesRead == 0)
    {
        ESP_LOGI(TAG, "0 bytes readed");
        Decoder_Close();
        return false;
    }

    g_ReadPtr = g_InputBuffer;
    g_BytesLeft = bytesRead;

    memset(&g_FrameInfo, 0, sizeof(g_FrameInfo));
    return true;
}

static DecoderResult Decoder_DecodeFrame(void)
{
    int result =
        MP3Decode(
            g_Decoder,
            &g_ReadPtr,
            &g_BytesLeft,
            g_PcmBuffer,
            0);

    switch(result)
    {
        case ERR_MP3_NONE:
            MP3GetLastFrameInfo(
                g_Decoder,
                &g_FrameInfo);
            
            if(!Output_IsStarted()){
                MediaOutputFormat format = {
                    .Channels = g_FrameInfo.nChans,
                    .SampleRate = g_FrameInfo.samprate,
                    .Bits = 16
                };
                 
                Output_SetFormat(format);
                Output_Start();
            }
            
            Player_UpdateTime(
                    g_FrameInfo.outputSamps / g_FrameInfo.nChans,
                    g_FrameInfo.samprate);
            
            if(!Output_Write(
                Decoder_GetPcmBuffer(),
                g_FrameInfo.outputSamps))
            {
                return DECODER_ERROR;
            }

            return DECODER_OK;

        case ERR_MP3_INVALID_FRAMEHEADER:

            if(feof(g_File))
            {
                ESP_LOGI(TAG, "Track finished");
                return DECODER_EOF;
            }
            break;

        default:
            break;
    }

    ESP_LOGE(
        TAG,
        "Decode error: %d",
        result);

    return DECODER_ERROR;
}

DecoderResult MediaDecoder_Step(void)
{
    if(g_BytesLeft < MP3_REFILL_THRESHOLD)
    {
        if(!FillBuffer())
        {
            if(g_BytesLeft == 0)
                return DECODER_EOF;
        }
    }

    return Decoder_DecodeFrame();
}

const MP3FrameInfo *Decoder_GetFrameInfo(void)
{
    return &g_FrameInfo;
}

const short *Decoder_GetPcmBuffer(void)
{
    return g_PcmBuffer;
}
static bool SkipMetadata(FILE *file)
{
    uint8_t header[10];

    if (fread(header, 1, sizeof(header), file) != sizeof(header))
        return false;

    if (memcmp(header, "ID3", 3) != 0)
    {
        fseek(file, 0, SEEK_SET);
        return true;
    }

    size_t tagSize =
        ((header[6] & 0x7F) << 21) |
        ((header[7] & 0x7F) << 14) |
        ((header[8] & 0x7F) << 7)  |
        (header[9] & 0x7F);

    if (fseek(file, tagSize + 10, SEEK_SET) != 0)
        return false;

    return true;
}

void Decoder_Close(void)
{
    if(g_File != NULL)
    {
        fclose(g_File);
        g_File = NULL;
    }

    if(g_Decoder != NULL)
    {
        MP3FreeDecoder(g_Decoder);
        g_Decoder = NULL;
    }

    g_ReadPtr = NULL;
    g_BytesLeft = 0;

    memset(&g_FrameInfo, 0, sizeof(g_FrameInfo));
    memset(g_InputBuffer, 0, sizeof(g_InputBuffer));
    memset(g_PcmBuffer, 0, sizeof(g_PcmBuffer));

    ESP_LOGI(TAG, "Playback closed");
}