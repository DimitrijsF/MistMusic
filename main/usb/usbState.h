#pragma once

typedef enum {
    USB_NODISK,
    USB_LOADING,
    USB_STOP,
    USB_PLAY
} UsbState;

UsbState UsbState_GetState(void);
void UsbState_Eject(void);
void UsbState_Loading(void);
void UsbState_Stop(void);
void UsbState_Play(void);