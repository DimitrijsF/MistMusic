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
#include <usb/usbStorage.h>

static const char *TAG = "CDC_PROTOCOL";
static LoadingState loadingState = STEP0;
static EjectingState ejectingState = STEP0;

#pragma region Protocol Packets
#pragma region Static Methods
static void SetLoadingState(LoadingState state);
static void SetEjectingState(EjectingState state);
static void SendDiskInfo(void);

static void HandleStatus(const uint8_t *packet);
static void Handle5101(const uint8_t *packet);
static void HandleStop(const uint8_t *packet);
static void HandleLoadingState(const uint8_t *packet);
static void HandlePlayModeRequest(const uint8_t *packet);
static void HandlePlayBack(const uint8_t *packet);
static void HandleTrackSelect(const uint8_t *packet);
static void HandleEjectRequest(const uint8_t *packet);
static void HandleDBRequest(const uint8_t *packet);
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
    {1, 1, {0x18}, HandleEjectRequest},
    {4, 4, {0xDB, 0x1D, 0x18, 0x02}, HandleDBRequest},
};
#pragma region Drive packets
static const uint8_t ProtoStatusDiskInSeq1[] = {
    0x72, 0x00, 0x6C
};
static const uint8_t ProtoStatusDiskInSeq2[] = {
    0x72, 0x00, 0x62
};
static const uint8_t ProtoStatusDiskEjectingSeq[] = {
    0x72, 0x00, 0x68
};
static const uint8_t ProtoStatusSTEP1[] = {
    0x72, 0x00, 0x02
};
static const uint8_t ProtoStatusSTEP2[] = {
    0x72, 0x01, 0x02
};
static const uint8_t ProtoStatusSTEP3[] = {
    0x72, 0x03, 0x02
};
static const uint8_t ProtoTocReady[] ={
    0x72, 0x07, 0x12
};
static const uint8_t ProtoStatusReadyToPlay[] = {
    0x72, 0x07, 0x32
};
static const uint8_t ProtoStatusNoDisk[] = {
    0x72, 0x00, 0x61
};
static const uint8_t ProtoStatusEjecting1[] = {
    0x72
};
static const uint8_t ProtoStatusEjecting2[] = {
    0x04, 0x22
};

static const uint8_t ProtoPreDiskInfo[] ={
    0x73, 0x3D, 0x03, 0x02
};
static const uint8_t ProtoPlayAnswerLoad[] = {
    0x15, 0x00, 0x00, 0x00, 0x00, 0x00
};
static const uint8_t ProtoPlayAnswerReady[] = {
    0x15, 0x00, 0x00, 0x00, 0x01, 0x00
};
#pragma endregion
static const uint8_t ProtoAnswer5101[] = {
    0x42, 0x04, 0x12
};
#pragma endregion

#pragma region PacketProcessors
void Handle5101(const uint8_t *packet){
    (void)packet;
    CdcUart_Send(ProtoAnswer5101, sizeof(ProtoAnswer5101)); 
}
void HandlePlayBack(const uint8_t *packet){
    CdcState state = GetCdcState();
    if(state != PLAY && state != STOP)
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
    {
        Player_Stop();
        CdcStopPlay();
    }
}
void HandlePlayModeRequest(const uint8_t *packet){
    (void)packet;
    CdcState state = GetCdcState();
    if(state == LOADING){
        switch (loadingState)
        {
            case STEP0: break;
            case STEP1:
                CdcUart_Send(ProtoStatusSTEP1, sizeof(ProtoStatusSTEP1)); 
                SetLoadingState(STEP2);
            break;
            case STEP2:
                CdcUart_Send(ProtoStatusSTEP2, sizeof(ProtoStatusSTEP2)); 
                vTaskDelay(pdMS_TO_TICKS(500));
                CdcUart_Send(ProtoStatusSTEP3, sizeof(ProtoStatusSTEP3)); 
                vTaskDelay(pdMS_TO_TICKS(500));
                CdcUart_Send(ProtoPreDiskInfo, sizeof(ProtoPreDiskInfo)); 
                vTaskDelay(pdMS_TO_TICKS(1000));
                SendDiskInfo();
                CdcUart_Send(ProtoTocReady, sizeof(ProtoTocReady)); 
                CdcUart_Send(ProtoPlayAnswerReady, sizeof(ProtoPlayAnswerReady));
                SetLoadingState(READY);
                SetEjectingState(INIT);
                CdcStopPlay();
            break;
            case READY:
                CdcUart_Send(ProtoStatusReadyToPlay, sizeof(ProtoStatusReadyToPlay));
            break;
            default:
            break;
        }
    }   
    if(state == STOP)
        CdcUart_Send(ProtoStatusReadyToPlay, sizeof(ProtoStatusReadyToPlay)); 
    if(state == NO_DISK)
        CdcUart_Send(ProtoStatusNoDisk, sizeof(ProtoStatusNoDisk)); 
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
        case STOP:
            CdcUart_Send(ProtoStatusReadyToPlay, sizeof(ProtoStatusReadyToPlay)); 
            break;
    }
}
void HandleLoadingState(const uint8_t *packet){
    (void)packet;
    if(GetCdcState() == LOADING){
        if(loadingState != READY)
            CdcUart_Send(ProtoPlayAnswerLoad, sizeof(ProtoPlayAnswerLoad));
        else
            CdcUart_Send(ProtoPlayAnswerReady, sizeof(ProtoPlayAnswerReady));
    }
    else if(GetCdcState() == STOP)
        CdcUart_Send(ProtoPlayAnswerReady, sizeof(ProtoPlayAnswerReady));
}
static void HandleEjectRequest(const uint8_t *packet){
    (void)packet;
    Player_Stop();
    CdcStopPlay();
    CdcEjectStart();
    CdcUart_Send(ProtoStatusEjecting1, sizeof(ProtoStatusEjecting1));
    CdcUart_Send(ProtoStatusEjecting2, sizeof(ProtoStatusEjecting2));
    SetEjectingState(FINISH);
    SetLoadingState(STEP0);
    UsbStorageEject();
    Player_Reset();
}
static void HandleDBRequest(const uint8_t *packet){
    (void)packet;
    if(GetCdcState() == EJECTING && ejectingState == FINISH){
        CdcUart_Send(ProtoStatusDiskInSeq2, sizeof(ProtoStatusDiskInSeq2));
        CdcUart_Send(ProtoStatusDiskInSeq1, sizeof(ProtoStatusDiskInSeq1));
        vTaskDelay(pdMS_TO_TICKS(1000));
        CdcUart_Send(ProtoStatusDiskEjectingSeq, sizeof(ProtoStatusDiskEjectingSeq));
        vTaskDelay(pdMS_TO_TICKS(1000));
        CdcUart_Send(ProtoStatusNoDisk, sizeof(ProtoStatusNoDisk));
        CdcNoDisk();
    }
}
#pragma endregion
static const char *LoadingStateToString(LoadingState state)
{
    switch (state)
    {
        case STEP0: return "STEP0";
        case STEP1:    return "STEP1";
        case STEP2: return "STEP2";
        case READY: return "READY";
        default: return "UNKNOWN";
    }
}
static const char *EjectingStateToString(LoadingState state)
{
    switch (state)
    {
        case INIT: return "INIT";
        case FINISH:    return "FINISH";
        default: return "UNKNOWN";
    }
}
static void SetLoadingState(LoadingState state){
    ESP_LOGI(TAG,
         "Loading state %s -> %s",
         LoadingStateToString(loadingState),
         LoadingStateToString(state));
    loadingState = state;
}
static void SetEjectingState(EjectingState state){
    ESP_LOGI(TAG,
         "Ejecting state %s -> %s",
         EjectingStateToString(ejectingState),
         EjectingStateToString(state));
    ejectingState = state;
}
void ProtocolDriveIn(void){
    CdcUart_Send(ProtoStatusDiskInSeq1, sizeof(ProtoStatusDiskInSeq1));
    vTaskDelay(pdMS_TO_TICKS(270));
    SetLoadingState(STEP1);
    CdcUart_Send(ProtoStatusDiskInSeq2, sizeof(ProtoStatusDiskInSeq2));
}
static void SendDiskInfo(void){
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

    // Special ACK for 23 00 00 05 -> E4 00 00 05 23
    if (length == 4 &&
        packet[0] == 0x23 &&
        packet[1] == 0x00 &&
        packet[2] == 0x00 &&
        packet[3] == 0x05)
    {
        const uint8_t reply[] =
        {
            0xE4,
            0x00,
            0x00,
            0x05,
            0x23
        };

        CdcUart_Send(reply, sizeof(reply));
        return;
    }
    
    if(length == 3 && packet[0] == 0x32){
        const uint8_t reply[] =
        {
            0xE3,
            packet[1],
            packet[2],
            packet[0]
        };
        CdcUart_Send(reply, sizeof(reply));
        return;
    }

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
        .Disc = 0x01,
        .Track = status.Track,
        .Reserved = 0x01
    };
    CdcUart_Send((uint8_t *)&packet, sizeof(packet));
}
void CdcProtocol_SendPlayReadyPacket(uint8_t track){
    PlayStatusPacket packet =
    {
        .Command = 0x15,
        .Minutes = 0x00,
        .Seconds = 0x00,
        .Disc = 0x00,
        .Track = track,
        .Reserved = 0x00
    };
    CdcUart_Send((uint8_t *)&packet, sizeof(packet));
}
void CdcProtocol_SendPlayStartPacket(uint8_t track){
    PlayStatusPacket packet =
    {
        .Command = 0x15,
        .Minutes = 0x00,
        .Seconds = 0x00,
        .Disc = 0x01,
        .Track = track,
        .Reserved = 0x01
    };
    CdcUart_Send((uint8_t *)&packet, sizeof(packet));
}
void CdcProtocol_SendStatusTocReady(void){
    CdcUart_Send(ProtoTocReady, sizeof(ProtoTocReady));
}
void CdcProtocol_SendStatusPlayReady(void){
    CdcUart_Send(ProtoStatusReadyToPlay, sizeof(ProtoStatusReadyToPlay));
}