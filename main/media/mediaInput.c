#include <stdbool.h>
#include <inttypes.h>
#include <stddef.h>
#include "esp_log.h"
#include "driver/i2s_std.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"

#include <mediaInput.h>

static const char *TAG = "MEDIA_INPUT";

static i2s_chan_handle_t InputChannel = NULL;
static bool IsInitialized = false;
static bool IsStarted = false;

static const gpio_num_t I2S_LRCK_GPIO = GPIO_NUM_40;
static const gpio_num_t I2S_BCK_GPIO  = GPIO_NUM_41;
static const gpio_num_t I2S_DATA_GPIO = GPIO_NUM_42;

bool Input_Init(void){
    if(IsInitialized)
        return true;

    i2s_chan_config_t channelConfig =
        I2S_CHANNEL_DEFAULT_CONFIG(
            I2S_NUM_1,
            I2S_ROLE_SLAVE);

    if(i2s_new_channel(
            &channelConfig,
            NULL,
            &InputChannel) != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to create I2S RX channel");
        return false;
    }

    i2s_std_config_t stdConfig =
    {
        .clk_cfg =
            I2S_STD_CLK_DEFAULT_CONFIG(44100),

        .slot_cfg =
            I2S_STD_MSB_SLOT_DEFAULT_CONFIG(
                I2S_DATA_BIT_WIDTH_32BIT,
                I2S_SLOT_MODE_STEREO),

        .gpio_cfg =
        {
            .mclk = I2S_GPIO_UNUSED,
            .bclk = I2S_BCK_GPIO,
            .ws = I2S_LRCK_GPIO,
            .dout = I2S_GPIO_UNUSED,
            .din = I2S_DATA_GPIO,

            .invert_flags =
            {
                .mclk_inv = false,
                .bclk_inv = false,
                .ws_inv = false,
            }
        }
    };

    stdConfig.slot_cfg.slot_bit_width =
        I2S_SLOT_BIT_WIDTH_32BIT;

    stdConfig.slot_cfg.bit_shift = false;
    stdConfig.slot_cfg.left_align = false;
    stdConfig.slot_cfg.ws_pol = true;
    stdConfig.slot_cfg.ws_width = 32;

    if(i2s_channel_init_std_mode(
            InputChannel,
            &stdConfig) != ESP_OK)
    {
        i2s_del_channel(InputChannel);
        InputChannel = NULL;
        return false;
    }

    IsInitialized = true;

    ESP_LOGI(TAG, "I2S input initialized");
    ESP_LOGI(
        TAG,
        "WS=%d BCLK=%d DIN=%d",
        I2S_LRCK_GPIO,
        I2S_BCK_GPIO,
        I2S_DATA_GPIO);

    return true;
}
bool Input_Start(void){
    if(!IsInitialized)
        return false;
    if(IsStarted)
        return true;
    if(i2s_channel_enable(InputChannel) != ESP_OK)
        return false;
    IsStarted = true;
    ESP_LOGI(TAG, "I2S input started");
    return true;
}
void Input_Stop(void){
    if(!IsStarted)
        return;
    i2s_channel_disable(InputChannel);
    IsStarted = false;
    ESP_LOGI(TAG, "I2S input stopped");
}
size_t Input_Read(int16_t *samples, size_t sampleCount){
    if(!IsStarted || samples == NULL || sampleCount == 0)
        return 0;

    size_t bytesRead = 0;
    esp_err_t err =
        i2s_channel_read(
            InputChannel,
            samples,
            sampleCount * sizeof(int16_t),
            &bytesRead,
            portMAX_DELAY);
    if(err != ESP_OK)
    {
        ESP_LOGE(
            TAG,
            "I2S read failed: %s",
            esp_err_to_name(err));

        return 0;
    }
    return bytesRead / sizeof(int16_t);
}
bool Input_IsStarted(void){
    return IsStarted;
}