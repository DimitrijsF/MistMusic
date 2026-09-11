#include <inttypes.h>
#include <stdbool.h>
#include <string.h>

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <bm83_uart.h>
#include <bm83_protocol.h>
#include <bm83_state.h>

//static const char *TAG = "BM83_PROTOCOL";

#pragma region Commands
static const BM_PACKET ProtoPowerOnPress = {
    .Command = 0x02,
    .Data = {0x00, 0x51},
    .DataLength = 2
};
static const BM_PACKET ProtoPowerOnRelease = {
    .Command = 0x02,
    .Data = {0x00, 0x52},
    .DataLength = 2
};
static const BM_PACKET ProtoSetPairing = {
    .Command = 0x02,
    .Data = {0x00, 0x5D},
    .DataLength = 2
};
static const BM_PACKET ProtoSetATRx = {
    .Command = 0x44,
    .Data = {0x03, 0x01},
    .DataLength = 2
};
#pragma endregion
static uint8_t CalculateChecksum(const uint8_t* data, size_t length)
{
    uint16_t sum = 0;
    for (size_t i = 0; i < length; i++)
        sum += data[i];
    return (uint8_t)(0x100 - (sum & 0xFF));
}
static BM_FRAME BuildPacket(const BM_PACKET* packet)
{
    BM_FRAME frame = { 0 };
    frame.Data[0] = 0xAA;
    frame.Data[1] = 0x00;
    frame.Data[2] = 1 + packet->DataLength;
    frame.Data[3] = packet->Command;
    if (packet->DataLength > 0)
        memcpy(&frame.Data[4], packet->Data, packet->DataLength);
    frame.Length = 4 + packet->DataLength;
    frame.Data[frame.Length] = CalculateChecksum(frame.Data, frame.Length);
    frame.Length++;
    return frame;
}
static void SendPacket(BM_PACKET* packet){
    BM_FRAME frame = BuildPacket(packet);
    BtUart_Send(frame.Data, frame.Length);
}
static void ProcessStatusEvent(uint8_t cmdData){
    switch (cmdData)
    {
        case 0x0F: //standby
            if(BtState_GetBtState() == BT_STARTING)
            {
                BtState_SetOn(); 
                if(!BtState_IsInitialized())
                {
                    SendPacket(&ProtoSetATRx);
                    BtState_SetInitDone();
                }
            }
            break;
        case 0x06: //A2DP connected (phone connected)
            if(BtState_GetBtState() == BT_POWERON || BtState_GetBtState() == BT_PAIRING){
                BtState_SetLinkConnected();
            }
            break; 
        case 0x15: //acl disconnected 
            if(BtState_GetLinkState() != LINK_DISCONNECTED)
                BtState_SetLinkDisconnected();
            break; 
        default: break;
    }
}
static void ProcessCallEvents(uint8_t cmdData){

}
void BtProto_ProcessPacket(const uint8_t *packet, uint8_t length){
    uint8_t eventId = packet[3];
    switch (eventId){
        case 0x01: ProcessStatusEvent(packet[4]); break;
        case 0x02: ProcessCallEvents(packet[4]); break;
    }
}
void BtProto_SendPowerOn(void){
    vTaskDelay(pdMS_TO_TICKS(5000));
    SendPacket(&ProtoPowerOnPress);
    SendPacket(&ProtoPowerOnRelease);
}
void BtProto_SendPairing(void){
    SendPacket(&ProtoSetPairing);
    BtState_EnablePairing();
}