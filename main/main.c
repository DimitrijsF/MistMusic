#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <usb/usbHost.h>
#include <usb/usbStorage.h>
#include <cdc/cdc.h>
#include <media/mediaOutput.h>

void app_main(void)
{
    cdcInit();
    UsbHost_Init();
    UsbStorage_Init();
    Output_Init();
    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}