#pragma once

#include "esp_err.h"
#include <stdio.h>

esp_err_t UsbStorage_Init(void);
void UsbStorageEject(void);
bool UsbStorage_DriveIn(void);
bool UsbStorage_FileExists(const char *path);
FILE *UsbStorage_OpenFile(const char *path);;