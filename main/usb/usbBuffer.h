#pragma once

#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stddef.h>

#include <usbLibrary.h>

#define USB_INDEX_BUFFER_GROWTH_STEP 32

typedef struct
{
    UsbTrack *Entries;
    size_t Count;
    size_t Capacity;
} UsbTrackIndexBuffer;

typedef enum
{
    USB_INDEX_COMPARE_ERROR = -1,
    USB_INDEX_COMPARE_DIFFERENT = 0,
    USB_INDEX_COMPARE_EQUAL = 1,
    USB_INDEX_COMPARE_NOT_FOUND = 2
} UsbIndexCompareResult;

extern UsbTrackIndexBuffer mainBuffer;
extern UsbTrackIndexBuffer plstBuffer;

void UsbBuffer_Init(UsbTrackIndexBuffer *buffer);
bool UsbBuffer_Add(UsbTrackIndexBuffer *buffer, const UsbTrack *track);
bool UsbBuffer_Save(const UsbTrackIndexBuffer *buffer, const char *path);
void UsbBuffer_Free(UsbTrackIndexBuffer *buffer);