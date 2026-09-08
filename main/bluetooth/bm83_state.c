#include <bm83_state.h>
#include <bm83_protocol.h>

#include <stdbool.h>
#include <esp_log.h>

static bool IsInitialized = false;
static BtState DeviceState = POWEROFF;
static BtLinkState LinkState = DISCONNECTED;

static const char *TAG = "BT_STATE";

BtLinkState BmState_GetLinkState(void){
    return LinkState;
}
BtState BmState_GetBtState(void){
    return DeviceState;
}
static const char *DeviceStateToString(BtState state)
{
    switch (state)
    {
        case STARTING: return "STARTING";
        case POWEROFF: return "POWEROFF";
        case POWERON: return "POWERON";
        case PAIRING: return "PAIRING";
        default: return "UNKNOWN";
    }
}
static const char *LinkStateToString(BtLinkState state)
{
    switch (state)
    {
        case DISCONNECTED: return "DISCONNECTED";
        case CONNECTED: return "CONNECTED";
        case STOPPED: return "STOPPED";
        case PLAYING: return "PLAYING";
        case PAUSE: return "PAUSE";
        case CALL: return "CALL";
        default: return "UNKNOWN";
    }
}
static void SetDeviceState(BtState state){
    ESP_LOGI(TAG,
         "BT state %s -> %s",
         DeviceStateToString(DeviceState),
         DeviceStateToString(state));
    DeviceState = state;
}
static void SetLinkState(BtLinkState state){
    ESP_LOGI(TAG,
         "Ejecting state %s -> %s",
         LinkStateToString(LinkState),
         LinkStateToString(state));
    LinkState = state;
}
bool BtState_IsInitialized(void){
    return IsInitialized;
}
void BtState_SetInitDone(void){
    IsInitialized = true;
}
void BtState_Enable(void){
    SetDeviceState(STARTING);
    BtProto_SendPowerOn();
}
void BtState_SetOn(void){
    SetDeviceState(POWERON);
}
void BtState_SetLinkConnected(void){
    SetLinkState(CONNECTED);
}