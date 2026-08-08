#include "cdc_uart.h"
#include <cdc_protocol.h>

#include "driver/uart.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include <inttypes.h>
#include <stdbool.h>

static const char *TAG = "CDC_UART";

#define CDC_UART_PORT      UART_NUM_1
#define CDC_UART_TX_PIN    GPIO_NUM_18
#define CDC_UART_RX_PIN    GPIO_NUM_17

#define CDC_UART_BAUDRATE  9600 

static void CdcUart_Task(void *arg);
static TaskHandle_t uartTaskHandle = NULL;

esp_err_t CdcUart_Init(void)
{
    uart_config_t config =
    {
        .baud_rate = CDC_UART_BAUDRATE,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_EVEN,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT
    };

    ESP_ERROR_CHECK(
        uart_driver_install(
            CDC_UART_PORT,
            1024,
            1024,
            0,
            NULL,
            0));

    ESP_ERROR_CHECK(
        uart_param_config(
            CDC_UART_PORT,
            &config));

    ESP_ERROR_CHECK(
        uart_set_pin(
            CDC_UART_PORT,
            CDC_UART_TX_PIN,
            CDC_UART_RX_PIN,
            UART_PIN_NO_CHANGE,
            UART_PIN_NO_CHANGE));

    xTaskCreate(
        CdcUart_Task,
        "CdcUart",
        4096,
        NULL,
        5,
        &uartTaskHandle);

    ESP_LOGI(TAG, "CDC UART initialized");
    ESP_LOGI(TAG,
         "UART=%d RX=%d TX=%d Baud=%d",
         CDC_UART_PORT,
         CDC_UART_RX_PIN,
         CDC_UART_TX_PIN,
         CDC_UART_BAUDRATE);
    return ESP_OK;
}

void UartShutDown(void)
{
    if (uartTaskHandle != NULL)
    {
        vTaskDelete(uartTaskHandle);
        uartTaskHandle = NULL;
    }

    ESP_ERROR_CHECK(
        uart_driver_delete(CDC_UART_PORT));
}

void CdcUart_Send(const uint8_t *data, size_t length)
{
    vTaskDelay(pdMS_TO_TICKS(20));

    uart_write_bytes(CDC_UART_PORT, data, length);

    char text[128];
    int pos = 0;

    for (size_t i = 0; i < length; i++)
    {
        pos += snprintf(
            text + pos,
            sizeof(text) - pos,
            "%02X ",
            data[i]);
    }

    ESP_LOGI(
        TAG,
        "TX (%u): >> %s",
        length,
        text);
}

static void CdcUart_Task(void *arg)
{
    uint8_t buffer[32];

    while (true)
    {
        int length =
            uart_read_bytes(
                CDC_UART_PORT,
                buffer,
                sizeof(buffer),
                pdMS_TO_TICKS(10));

        if (length > 0)
        {
            char text[256];
            int pos = 0;
            for (int i = 0; i < length; i++)
            {
                pos += snprintf(
                    text + pos,
                    sizeof(text) - pos,
                    "%02X ",
                    buffer[i]);
            }
            ESP_LOGI(
                TAG,
                "RX (%d): << %s",
                length,
                text);
            CdcProtocol_ProcessPacket(buffer, length);
        }
    }
}