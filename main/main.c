#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <usb/usbHost.h>
#include <usb/usbStorage.h>
#include <cdc/cdc.h>
#include <media/mediaOutput.h>
#include <bluetooth/bm83.h>
#include <wifi/wifiService.h>

void app_main(void)
{
    WifiService_Init();
    cdcInit();
    UsbHost_Init();
    UsbStorage_Init();
    Output_Init();
    BT_Init();
    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}