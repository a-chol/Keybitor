/*++

Copyright (c) Microsoft Corporation.  All rights reserved.

    THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
    KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
    PURPOSE.

Module Name:

    KBFTEST.C

Abstract:


Environment:

    usermode console application

--*/

#include <wtypes.h>
#include <Keybitor.h>



typedef struct {
  wchar_t* requested_device;
  int index;
} context;

void display_device(const wchar_t* device, void* data){
  context* ctx = (context*) data;

  printf("%3d:    %5s  %s\n", ++(ctx->index), (isDeviceEnable(device)==1?"ON":"OFF"), device);

  if (ctx->requested_device == NULL || wcslen(ctx->requested_device) == 0 || wcscmp(device, ctx->requested_device) == 0){
    toggleDevice(device);
  }
}

int
_cdecl
main(
    int argc,
    char *argv[]
    )
{
    
    wchar_t* providedDeviceId = NULL;

    if (argc > 1 && strlen(argv[1])>0){
      size_t devSize = strlen(argv[1]) + 1;
      providedDeviceId = malloc(sizeof(WCHAR) * (devSize));
      if (mbstowcs(providedDeviceId, argv[1], devSize) == -1){
        free(providedDeviceId);
        providedDeviceId = NULL;
      }
    }

    context ctx;
    ctx.index = 0;
    ctx.requested_device = providedDeviceId;

    printf("\nPS2KeyboardToggler : Switch PS2 Keyboard bypass state\n");
    printf("-------------------------------------------------------\n");
    printf("Device list:\n");
    printf("Index:  DeviceId\n");

    int i = 0;

    foreachDevice(display_device, (void*) &ctx);

    return 0;
}
