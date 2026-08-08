#pragma once

#include <inttypes.h>

#include <media/mediaPlayer.h>

typedef struct
{
    uint8_t Command;
    uint8_t LeadOut1;
    uint8_t LeadOut2;
    uint8_t LeadOut3;
    uint8_t Tracks;
    uint8_t Unknown;
    uint8_t Reserved;

} TocPacket;

typedef struct{
    uint8_t Command;
    uint8_t Seconds;
    uint8_t Minutes;
    uint8_t Track;
    uint8_t Reserved;
} PlayStatusPacket;

void CdcProtocol_ProcessPacket(const uint8_t *packet, uint8_t length);
void CdcProtocol_SendAck(const uint8_t *packet, uint8_t length);
void StartDriveInSequence(void);
void DriveInReading(void);
void DriveInComplete(void);
void CdcProtocol_SendPlayStatus(PlayStatus status);