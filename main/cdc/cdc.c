#include <stdbool.h>

#include <cdc/cdc_uart.h>
#include <cdc/cdc_state.h>

#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define CDC_POWER_ON_PIN  GPIO_NUM_10

static void CdcStateTask(void *arg);
static void CdcPowerPinInit(void);
static bool IsHeadEnabled(void);

static void CdcStateTask(void *arg){
    while (true)
    {  
        if (GetCdcState() == STANDBY)
        {
            if(IsHeadEnabled()){
                CdcBoot();
            }
        }
        else{
            if(!IsHeadEnabled()){
                CdcStandby();
            }
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
static void CdcPowerPinInit(void){
    gpio_config_t ioConfig =
    {
        .pin_bit_mask = 1ULL << CDC_POWER_ON_PIN,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    ESP_ERROR_CHECK(gpio_config(&ioConfig));
}
static bool IsHeadEnabled(void){
    return gpio_get_level(CDC_POWER_ON_PIN) == 1;
}

void cdcInit(void){
    CdcPowerPinInit();
    xTaskCreate(
    CdcStateTask,
    "CdcState",
    4096,
    NULL,
    5,
    NULL);
}