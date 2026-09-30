#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <manager/playerManager.h>

void app_main(void)
{
    PlayerManager_Init();
    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}