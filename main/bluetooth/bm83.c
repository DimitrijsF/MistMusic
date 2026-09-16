#include <bm83_uart.h>
#include <bm83_state.h>

#include <stdbool.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void BT_Init(void){
    BtUart_Init();
    vTaskDelay(pdMS_TO_TICKS(500));
    BtState_Enable();
}