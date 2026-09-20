#pragma once

#include <stdbool.h>

void Power_Init(void);
void Power_Enable(void);
void Power_Standby(void);
bool Power_IsOn(void);