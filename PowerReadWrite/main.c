#include "main.h"


// DriverUnload
VOID DriverUnload(__in PDRIVER_OBJECT pDriverObject)
{
	LOGDEBUG(DriverUnload, "DriverUnload Called");

	// Delete Device LinkName and Device Name
	UNICODE_STRING usDeviceSymLink;
	RtlInitUnicodeString(&usDeviceSymLink, L"\\??\\PowerLinkName");
	IoDeleteSymbolicLink(&usDeviceSymLink);
	IoDeleteDevice(pDriverObject->DeviceObject);

	return;
}

// DriverEntry
NTSTATUS DriverEntry(__in PDRIVER_OBJECT pDriverObject, __in PUNICODE_STRING pRegistryPath)
{
	UNREFERENCED_PARAMETER(pRegistryPath);

	LOGDEBUG(DriverEntry, "DriverEntry Called");

	// Register Unload Function
	pDriverObject->DriverUnload = DriverUnload;

	// Create Device
	UNICODE_STRING usDeviceName;
	RtlInitUnicodeString(&usDeviceName, DEVICE_NAME);
	PDEVICE_OBJECT pDeviceObject = NULL;
	NTSTATUS status = IoCreateDevice(
		pDriverObject,					// Driver Object
		0,								// Extend device size
		&usDeviceName,					// Device Name
		FILE_DEVICE_UNKNOWN,			// Device Type
		FILE_DEVICE_SECURE_OPEN,		// Device Characteristics
		FALSE,							// Is it exclusive
		&pDeviceObject);				// Device Object -- OUT
	if (!NT_SUCCESS(status))
	{
		LOGDEBUG(status, "CreateDevice Fail status");
		return status;
	}

	// Create SymbolLink Name
	UNICODE_STRING usDeviceSymlink;
	RtlInitUnicodeString(&usDeviceSymlink, DEVICE_LINKNAME);
	status = IoCreateSymbolicLink(&usDeviceSymlink, &usDeviceName);
	if (!NT_SUCCESS(status))
	{
		LOGDEBUG(status, "CreateSymbolicLink Fail status");
		IoDeleteDevice(pDeviceObject);
		return status;
	}

	KdPrint(("[Power]->[0x%p]->DeviceName:%wZ\n", POWER_SUCCESS, usDeviceName));
	KdPrint(("[Power]->[0x%p]->SyLinkName:%wZ\n", POWER_SUCCESS, usDeviceSymlink));

	// Fill IRP Dispatch Function
	for (size_t i = 0; i < IRP_MJ_MAXIMUM_FUNCTION; i++)
	{
		pDriverObject->MajorFunction[i] = DeviceDefaultDispatch;
	}
	pDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DeviceControlDispatch;

	return status;
}