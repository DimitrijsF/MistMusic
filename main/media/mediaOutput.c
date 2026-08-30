#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include <esp_log.h>
#include "freertos/FreeRTOS.h"
#include <driver/i2s_std.h>
#include <driver/gpio.h>

#include <mediaOutput.h>

#pragma region Static Methods
static i2s_data_bit_width_t ToBitWidth(uint8_t bits);
static i2s_slot_mode_t ToSlotMode(uint8_t channels);
#pragma endregion

static const gpio_num_t I2S_LRCK_GPIO = GPIO_NUM_4;
static const gpio_num_t I2S_DATA_GPIO = GPIO_NUM_5;
static const gpio_num_t I2S_BCK_GPIO  = GPIO_NUM_6;

static const char *TAG = "MEDIA_OUTPUT";

static i2s_chan_handle_t OutputChannel = NULL;

static bool IsInitialized = false;
static bool IsStarted = false;

static MediaOutputFormat OutputFormat;
bool Output_Init(void)
{
    if(IsInitialized)
        return true;

    i2s_chan_config_t channelConfig =
        I2S_CHANNEL_DEFAULT_CONFIG(
            I2S_NUM_AUTO,
            I2S_ROLE_MASTER);

    if(i2s_new_channel(
            &channelConfig,
            &OutputChannel,
            NULL) != ESP_OK)
    {
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
            .dout = I2S_DATA_GPIO,
            .din = I2S_GPIO_UNUSED,
            .invert_flags =
            {
                .mclk_inv = false,
                .bclk_inv = false,
                .ws_inv = false,
            }
        }
    };

    stdConfig.slot_cfg.slot_bit_width = I2S_SLOT_BIT_WIDTH_32BIT;
    stdConfig.slot_cfg.bit_shift = false;
    stdConfig.slot_cfg.left_align = false;
    stdConfig.slot_cfg.ws_pol = true;
    stdConfig.slot_cfg.ws_width = 32;

    if(i2s_channel_init_std_mode(
            OutputChannel,
            &stdConfig) != ESP_OK)
    {
        i2s_del_channel(OutputChannel);
        OutputChannel = NULL;
        return false;
    }

    IsInitialized = true;

    ESP_LOGI(TAG, "Media output initialized");

    return true;
}

void Output_SetFormat(MediaOutputFormat format){
    OutputFormat = format;
}

bool Output_Start(void)
{
   if(IsStarted)
        return true;

    ESP_LOGI(TAG, "Changing format...");
    i2s_std_clk_config_t clkConfig =
        I2S_STD_CLK_DEFAULT_CONFIG(
            OutputFormat.SampleRate);

    ESP_ERROR_CHECK(
        i2s_channel_reconfig_std_clock(
            OutputChannel,
            &clkConfig));

    i2s_std_slot_config_t slotConfig =
        I2S_STD_MSB_SLOT_DEFAULT_CONFIG(
            ToBitWidth(OutputFormat.Bits),
            ToSlotMode(OutputFormat.Channels));

    slotConfig.slot_bit_width = I2S_SLOT_BIT_WIDTH_32BIT;
    slotConfig.bit_shift = false;
    slotConfig.left_align = false;
    slotConfig.ws_pol = true;
    slotConfig.ws_width = 32;
    ESP_ERROR_CHECK(
        i2s_channel_reconfig_std_slot(
            OutputChannel,
            &slotConfig));

    i2s_channel_enable(
            OutputChannel);

    IsStarted = true;

    ESP_LOGI(
        TAG,
        "Started %lu Hz, %u ch, %u bit",
        (unsigned long)OutputFormat.SampleRate,
        OutputFormat.Channels,
        OutputFormat.Bits);

    return true;
}

void Output_Stop(void){
    if(!IsStarted)
        return;
    IsStarted = false;
    ESP_ERROR_CHECK(
    i2s_channel_disable(
        OutputChannel));
    
    ESP_LOGI(TAG, "Media output stopped");
}

bool Output_Write(const int16_t *samples, size_t sampleCount){
    if(!IsStarted)
        return false;

    size_t bytesWritten = 0;

    esp_err_t err =
        i2s_channel_write(
            OutputChannel,
            samples,
            sampleCount * sizeof(int16_t),
            &bytesWritten,
            portMAX_DELAY);

    if(err != ESP_OK)
    {
        ESP_LOGE(
            TAG,
            "I2S write failed: %s",
            esp_err_to_name(err));

        return false;
    }

    return bytesWritten == sampleCount * sizeof(int16_t);
}

static i2s_data_bit_width_t ToBitWidth(uint8_t bits)
{
    switch(bits)
    {
        case 8:
            return I2S_DATA_BIT_WIDTH_8BIT;

        case 16:
            return I2S_DATA_BIT_WIDTH_16BIT;

        case 24:
            return I2S_DATA_BIT_WIDTH_24BIT;

        case 32:
            return I2S_DATA_BIT_WIDTH_32BIT;

        default:
            return I2S_DATA_BIT_WIDTH_16BIT;
    }
}
static i2s_slot_mode_t ToSlotMode(uint8_t channels){
    switch (channels)
    {
        case 1:
            return I2S_SLOT_MODE_MONO;
        case 2:
            return I2S_SLOT_MODE_STEREO;
        default:
            return I2S_SLOT_MODE_MONO;
    }
}
void Output_Silence(void)
{
    if(!IsStarted)
        return;

    static const int16_t silence[2048] = {0};

    size_t bytesWritten;
    ESP_LOGI(TAG, "Writing silence...");
    i2s_channel_write(
        OutputChannel,
        silence,
        sizeof(silence),
        &bytesWritten,
        portMAX_DELAY);
}
bool Output_IsStarted(void){
    return IsStarted;
}