#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

typedef struct
{
    uint32_t SampleRate;
    uint8_t Channels;
    uint8_t Bits;
} MediaOutputFormat;

bool Output_Init(void);
void Output_SetFormat(MediaOutputFormat format);
bool Output_Start(void);
void Output_Stop(void);
bool Output_Write(const int16_t *samples, size_t sampleCount);
bool Output_IsStarted(void);
void Output_Silence(void);