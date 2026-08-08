#pragma once

#include "esp_err.h"
#include <inttypes.h>

esp_err_t CdcUart_Init(void);
void CdcUart_Send(const uint8_t *data, size_t length);
void UartShutDown(void);