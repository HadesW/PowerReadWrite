#pragma once
#include "main.h"
#include "function.h"

#define IOCTL_POWER_OPENPROCESS CTL_CODE(FILE_DEVICE_UNKNOWN, 0x801, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_POWER_GRANT_ACCESS CTL_CODE(FILE_DEVICE_UNKNOWN, 0x802, METHOD_BUFFERED, FILE_ANY_ACCESS)


NTSTATUS DeviceDefaultDispatch(__in PDEVICE_OBJECT pDeviceObject, __in PIRP pIrp);
NTSTATUS DeviceControlDispatch(__in PDEVICE_OBJECT pDeviceObject, __in PIRP pIrp);
