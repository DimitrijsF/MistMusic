#pragma once

#include <inttypes.h>

typedef struct
{
    uint8_t Command;
    uint8_t Data[8];  
    uint8_t DataLength;
} BM_PACKET;

typedef struct
{
    uint8_t Data[16];
    uint8_t Length;
} BM_FRAME;

void BtProto_ProcessPacket(const uint8_t *packet, uint8_t length);
void BtProto_SendPowerOn(void);