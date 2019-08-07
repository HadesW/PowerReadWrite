#pragma once
#include "main.h"
#include <tchar.h>


#define DEVICE_LINKNAME _T("\\??\\PowerLinkName")
#define IOCTL_POWER_SET_PROCESS 0x801
#define IOCTL_POWER_GRANT_ACCESS 0x802
#define IOCTL_POWER_READ_MEMORY 0x803
#define IOCTL_POWER_WRITE_MEMORY 0x804

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