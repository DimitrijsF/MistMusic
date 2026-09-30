#include <usbState.h>
#include <string.h>
#include <esp_log.h>
#include <stdbool.h>

static const char *TAG = "USB_STATE";
static bool UsbRandom = false;
static UsbState State = USB_NODISK;

UsbState UsbState_GetState(void){
    return State;
}
static const char *UsbStateToString(UsbState state)
{
    switch (state)
    {
        case USB_NODISK: return "USB_NODISK";
        case USB_LOADING: return "USB_LOADING";
        case USB_STOP: return "USB_STOP";
        case USB_PLAY: return "USB_PLAY";
        default: return "UNKNOWN";
    }
}
static void SetUsbState(UsbState state){
    if(state != State){
        UsbState current = State;
        State = state;
        ESP_LOGI(TAG, "USB state changed %s -> %s", UsbStateToString(current), UsbStateToString(state));
    }
}
void UsbState_Eject(void){
    SetUsbState(USB_NODISK);
}
void UsbState_Loading(void){
    SetUsbState(USB_LOADING);
}
void UsbState_Stop(void){
    SetUsbState(USB_STOP);
}
void UsbState_Play(void){
    SetUsbState(USB_PLAY);
}
void UsbState_SetUsbRandom(bool value){
    UsbRandom = value;
}
bool UsbState_GetUsbRandom(void){
    return UsbRandom;
}