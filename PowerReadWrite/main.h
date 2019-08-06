#pragma once
#include "ntifs.h"
#include "dispatch.h"


#if DBG
#define INT3BREAKPOINT KdBreakPoint();
#define LOGDEBUG(var,message) DbgPrint("[Power]->[0x%p]->%s\n",var,message);
#else
#define INT3BREAKPOINT
#define LOGDEBUG(var,message)
#endif

#define POWER_SUCCESS 1
#define POWER_UNSUCCESSFUL 0

#define DEVICE_NAME  L"\\Device\\PowerDeviceName"
#define DEVICE_LINKNAME L"\\??\\PowerLinkName"


