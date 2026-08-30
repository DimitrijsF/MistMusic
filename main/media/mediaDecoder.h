#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <mp3dec.h>

typedef enum
{
    DECODER_OK,
    DECODER_EOF,
    DECODER_ERROR
} DecoderResult;

bool Decoder_Open(uint16_t trackNumber);
void Decoder_Close(void);
long Decoder_GetPosition(void);
bool Decoder_OpenAt(uint16_t trackNumber, long position);

DecoderResult MediaDecoder_Step(void);

const MP3FrameInfo *Decoder_GetFrameInfo(void);
const short *Decoder_GetPcmBuffer(void);

long Decoder_GetTrackSize(uint16_t trackNumber);