#include <inttypes.h>
#include <stdbool.h>
#include <string.h>

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <cdc/cdc_protocol.h>
#include <cdc/cdc_uart.h>
#include <cdc/cdc_state.h>
#include <media/mediaLibrary.h>
#include <media/mediaPlayer.h>

static const char *TAG = "CDC_PROTOCOL";

#pragma region Protocol Packets
#pragma region Static Methods
static void HandleStatus(const uint8_t *packet);
static void Handle5101(const uint8_t *packet);
static void HandleStop(const uint8_t *packet);
static void HandleLoadingState(const uint8_t *packet);
static void HandlePlayModeRequest(const uint8_t *packet);
static void HandlePlayBack(const uint8_t *packet);
static void HandleTrackSelect(const uint8_t *packet);
#pragma endregion
typedef void (*CdcPacketHandler)(const uint8_t *packet);

typedef struct
{
    uint8_t Length;
    uint8_t CompareLength;
    uint8_t Packet[8];  
    CdcPacketHandler Handler;

} CdcCommand;

static const CdcCommand Commands[] =
{
    {1, 1, {0x60}, HandleStatus},
    {2, 2, {0x51, 0x01}, Handle5101},
    {2, 2, {0x09, 0x00}, HandleStop},
    {2, 2, {0x79, 0x03}, HandleLoadingState},
    {2, 2, {0x09, 0x02}, HandlePlayModeRequest},
    {4, 3, {0x23, 0x00, 0x00}, HandlePlayBack},
    {3, 2, {0x32, 0x01}, HandleTrackSelect},
};
#pragma region Drive State
static const uint8_t ProtoStatusNoDisk[] = {
    0x72, 0x00, 0x61
};
static const uint8_t ProtoStatusDiskLoading[] ={
    0x72, 0x01, 0x02
};
static const uint8_t ProtoStatusDiskReaded[] = {
    0x72, 0x03, 0x02
};
static const uint8_t ProtoStatusDiskInSeq1[] = {
    0x72, 0x00, 0x6C
};
static const uint8_t ProtoStatusDiskInSeq2[] = {
    0x72, 0x00, 0x62
};
static const uint8_t ProtoStatusReadyToPlay[] = {
    0x72, 0x07, 0x32
};
static const uint8_t ProtoTocReady[] ={
    0x72, 0x07, 0x12
};
#pragma endregion
static const uint8_t ProtoAnswer5101[] = {
    0x42, 0x04, 0x12
};
static const uint8_t ProtoPlayAnswerLoad[] = {
    0x15, 0x00, 0x00, 0x00, 0x00, 0x00
};
static const uint8_t ProtoPlayAnswerReady[] = {
    0x15, 0x00, 0x00, 0x00, 0x01, 0x00
};
static const uint8_t ProtoPreDiskInfo[] ={
    0x73, 0x3D, 0x03, 0x02
};
#pragma endregion

#pragma region PacketProcessors
void Handle5101(const uint8_t *packet){
    (void)packet;
    CdcUart_Send(ProtoAnswer5101, sizeof(ProtoAnswer5101)); 
}
void HandlePlayBack(const uint8_t *packet){
    CdcState state = GetCdcState();
    if(state != READY && state != PLAY && state != STOP)
        return;
        
    switch(packet[3])
    {
        case 0x00:
            Player_Play();
            break;

        case 0x20:
            Player_FF(false);
            break;

        case 0x60:
            Player_FF(true);
            break;

        case 0xA0:
            Player_Rew(false);
            break;

        case 0xE0:
            Player_Rew(true);
            break;
    }
}

void HandleTrackSelect(const uint8_t *packet){
    uint8_t track = packet[2];
    Player_SwitchTrack(track);
}
void HandleStop(const uint8_t *packet){
    (void)packet;
    if(GetCdcState() == PLAY)
        Player_Stop();
}
void HandlePlayModeRequest(const uint8_t *packet){
    (void)packet;
    CdcState state = GetCdcState();
    if(state == LOADING)
        CdcUart_Send(ProtoStatusDiskLoading, sizeof(ProtoStatusDiskLoading)); 
    if(state == READY)
        CdcUart_Send(ProtoStatusReadyToPlay, sizeof(ProtoStatusReadyToPlay)); 
}
void HandleStatus(const uint8_t *packet)
{
    (void)packet;
    switch(GetCdcState())
    {
        case NO_DISK:
            CdcUart_Send(ProtoStatusNoDisk, sizeof(ProtoStatusNoDisk)); 
            break;

        case STANDBY: break;
        case BOOT:    break;
        case LOADING: break;
        case EJECTING: break;
        case PLAY: break;
        case STOP: break; 
        case READY: 
            CdcUart_Send(ProtoStatusReadyToPlay, sizeof(ProtoStatusReadyToPlay)); 
            break;
    }
}
void HandleLoadingState(const uint8_t *packet){
    (void)packet;
    if(GetCdcState() == LOADING)
        CdcUart_Send(ProtoPlayAnswerLoad, sizeof(ProtoPlayAnswerLoad));
    if(GetCdcState() == READY)
        CdcUart_Send(ProtoPlayAnswerReady, sizeof(ProtoPlayAnswerReady));
}
#pragma endregion

void StartDriveInSequence(void){
    CdcUart_Send(ProtoStatusDiskInSeq1, sizeof(ProtoStatusDiskInSeq1));
}

void DriveInReading(void){
    CdcUart_Send(ProtoStatusDiskInSeq2, sizeof(ProtoStatusDiskInSeq2));
}
void DriveInComplete(void){
    CdcUart_Send(ProtoStatusDiskReaded, sizeof(ProtoStatusDiskReaded));
    CdcUart_Send(ProtoPreDiskInfo, sizeof(ProtoPreDiskInfo));
    TocPacket packet =
    {
        .Command = 0x36,
        .LeadOut1 = 0x05,
        .LeadOut2 = 0x1D,
        .LeadOut3 = 0x40,
        .Tracks = MediaLibrary_GetCount(),
        .Unknown = 0x01,
        .Reserved = 0x00
    };
    CdcUart_Send((uint8_t *)&packet, sizeof(packet));
    CdcUart_Send(ProtoTocReady, sizeof(ProtoTocReady));
    CdcUart_Send(ProtoPlayAnswerReady, sizeof(ProtoPlayAnswerReady));
}

void CdcProtocol_ProcessPacket(const uint8_t *packet, uint8_t length)
{
    CdcProtocol_SendAck(packet, length);
    for(size_t i = 0; i < sizeof(Commands) / sizeof(Commands[0]); i++)
    {
        if(Commands[i].Length != length)
            continue;

        if(memcmp(packet, Commands[i].Packet, Commands[i].CompareLength) != 0)
            continue;
        
        Commands[i].Handler(packet);
        return;
    }
}

void CdcProtocol_SendAck(const uint8_t *packet, uint8_t length)
{
    if (length == 0)
        return;

    bool allZero = true;
    bool allFF = true;

    for (uint8_t i = 0; i < length; i++)
    {
        if (packet[i] != 0x00)
            allZero = false;

        if (packet[i] != 0xFF)
            allFF = false;
    }

    if (allZero)
    {
        ESP_LOGD(TAG, "Ignore: all zero");
        return;
    }

    if (allFF)
    {
        ESP_LOGD(TAG, "Ignore: all AA");
        return;
    }

    uint8_t reply[16];

    reply[0] = 0xE0 | length;

    if (packet[0] == 0xDB)
    {
        memcpy(&reply[1], &packet[1], length - 1);

        reply[length] = 0xDB;
    }
    else
    {
        for (uint8_t i = 0; i < length; i++)
        {
            reply[i + 1] = packet[length - 1 - i];
        }
    }
    CdcUart_Send(reply, length + 1);
}

void CdcProtocol_SendPlayStatus(PlayStatus status){
    PlayStatusPacket packet =
    {
        .Command = 0x15,
        .Minutes = status.Minutes,
        .Seconds = status.Seconds,
        .Track = status.Track,
        .Reserved = 0x01
    };
    CdcUart_Send((uint8_t *)&packet, sizeof(packet));
}