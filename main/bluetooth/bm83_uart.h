#pragma once

#include "esp_err.h"
#include <inttypes.h>

esp_err_t BtUart_Init(void);
void BtUart_Send(const uint8_t *data, size_t length);