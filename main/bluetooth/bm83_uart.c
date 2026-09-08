#include "driver/uart.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include <inttypes.h>
#include <stdbool.h>

#include <bm83_protocol.h>

static const char *TAG = "BT_UART";

#define BT_UART_PORT      UART_NUM_2
#define BT_UART_TX_PIN    GPIO_NUM_35
#define BT_UART_RX_PIN    GPIO_NUM_36

#define BT_UART_BAUDRATE  115200

static void BtUart_Task(void *arg);

esp_err_t BtUart_Init(void)
{
    uart_config_t config =
    {
        .baud_rate = BT_UART_BAUDRATE,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT
    };

    ESP_ERROR_CHECK(
        uart_driver_install(
            BT_UART_PORT,
            1024,
            1024,
            0,
            NULL,
            0));

    ESP_ERROR_CHECK(
        uart_param_config(
            BT_UART_PORT,
            &config));

    ESP_ERROR_CHECK(
        uart_set_pin(
            BT_UART_PORT,
            BT_UART_TX_PIN,
            BT_UART_RX_PIN,
            UART_PIN_NO_CHANGE,
            UART_PIN_NO_CHANGE));

    xTaskCreate(
        BtUart_Task,
        "CdcUart",
        4096,
        NULL,
        5,
        NULL);

    ESP_LOGI(TAG, "UART initialized");
    ESP_LOGI(TAG,
         "UART=%d RX=%d TX=%d Baud=%d",
         BT_UART_PORT,
         BT_UART_RX_PIN,
         BT_UART_TX_PIN,
         BT_UART_BAUDRATE);
    return ESP_OK;
}

void BtUart_Send(const uint8_t *data, size_t length)
{
    vTaskDelay(pdMS_TO_TICKS(50));
    uart_write_bytes(BT_UART_PORT, data, length);

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

static void BtUart_Task(void *arg)
{
    uint8_t buffer[32];

    while (true)
    {
        int length =
            uart_read_bytes(
                BT_UART_PORT,
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
         /*   ESP_LOGI(
                TAG,
                "RX (%d): << %s",
                length,
                text);*/
            BtProto_ProcessPacket(buffer, length);
        }
    }
}