#include <bm83_state.h>
#include <bm83_protocol.h>

#include <sourceManager/sourceManager.h>

#include <stdbool.h>
#include <esp_log.h>

static bool IsInitialized = false;
static BtState DeviceState = BT_POWEROFF;
static BtLinkState LinkState = LINK_DISCONNECTED;

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
        case BT_STARTING: return "BT_STARTING";
        case BT_POWEROFF: return "BT_POWEROFF";
        case BT_POWERON: return "BT_POWERON";
        case BT_PAIRING: return "BT_PAIRING";
        default: return "UNKNOWN";
    }
}
static const char *LinkStateToString(BtLinkState state)
{
    switch (state)
    {
        case LINK_DISCONNECTED: return "LINK_DISCONNECTED";
        case LINK_CONNECTED: return "LINK_CONNECTED";
        case LINK_STOP: return "LINK_STOP";
        case LINK_PLAY: return "LINK_PLAY";
        case LINK_PAUSE: return "LINK_PAUSE";
        case LINK_CALL: return "LINK_CALL";
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
    SetDeviceState(BT_STARTING);
    BtProto_SendPowerOn();
}
void BtState_SetOn(void){
    SetDeviceState(BT_POWERON);
}
void BtState_SetLinkConnected(void){
    SrcManager_CheckSource();
    SetLinkState(LINK_CONNECTED);
}
void BtState_SetLinkDisconnected(void){
    SrcManager_CheckSource();
    SetLinkState(LINK_DISCONNECTED);
}
void BtState_EnablePairing(void){
    SetDeviceState(BT_PAIRING);
}