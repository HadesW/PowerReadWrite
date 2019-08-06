#pragma once
#include "main.h"
#include <tchar.h>


#define DEVICE_LINKNAME _T("\\??\\PowerLinkName")
#define IOCTL_POWER_SET_PROCESS 0x801

VOID GetSysFullPath(PTCHAR wsSysFileName);

BOOL Install();
BOOL Start();
BOOL Stop();
BOOL Uninstall();
BOOL OpenDevice(PTCHAR wsDevLinkName);
BOOL CloseDevice();
BOOL IoCtrlDriver(DWORD dwIoCode, PVOID pInBuffer, DWORD dwInBufferLen, PVOID pOutBuffer, DWORD dwOutBufferLen, DWORD *RealRetBytes);

BOOL LoadDriver();
BOOL UnloadDriver();