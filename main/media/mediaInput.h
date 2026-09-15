#pragma once

#include <stdbool.h>
#include <inttypes.h>
#include <string.h>

bool Input_Init(void);
bool Input_Start(void);
void Input_Stop(void);
size_t Input_Read(int16_t *samples, size_t sampleCount);
bool Input_IsStarted(void);