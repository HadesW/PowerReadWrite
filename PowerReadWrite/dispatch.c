#include "dispatch.h"

NTSTATUS DeviceDefaultDispatch(__in PDEVICE_OBJECT pDeviceObject, __in PIRP pIrp)
{
	UNREFERENCED_PARAMETER(pDeviceObject);
	// Set IRP status
	pIrp->IoStatus.Status = STATUS_SUCCESS;
	// Set IRP Operation Byte
	pIrp->IoStatus.Information = 0;
	// Request IRP
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

NTSTATUS DeviceControlDispatch(
	__in PDEVICE_OBJECT pDeviceObject,
	__in PIRP pIrp
)
{
	UNREFERENCED_PARAMETER(pDeviceObject);

	//KdBreakPoint();

	// retn status
	NTSTATUS status = STATUS_INVALID_DEVICE_REQUEST;
	// Get IRP Stack
	PIO_STACK_LOCATION pIrpStack = IoGetCurrentIrpStackLocation(pIrp);
	// Get IoControlCode
	ULONG ulIoControlCode = pIrpStack->Parameters.DeviceIoControl.IoControlCode;
	// Get SystemBuffer
	PVOID pIoBuffer = pIrp->AssociatedIrp.SystemBuffer;
	// Get Input Size
	ULONG ulInputSize = pIrpStack->Parameters.DeviceIoControl.InputBufferLength;
	// Get Output Size
	ULONG ulOutputSize = pIrpStack->Parameters.DeviceIoControl.OutputBufferLength;
	// Dispatch
	switch (ulIoControlCode)
	{
	case IOCTL_POWER_OPENPROCESS:
	{
		if (ulInputSize >= sizeof(OPENPROCESS_DATA) && pIoBuffer)
			status = PowerOpenProcess((POPENPROCESS_DATA)pIoBuffer);
		else
			status = STATUS_INFO_LENGTH_MISMATCH;
	}
	break;
	case IOCTL_POWER_GRANT_ACCESS:
	{
		if (ulInputSize >= sizeof(HANDLE_GRANT_ACCESS_DATA) && pIoBuffer)
			status = PowerGrantAccess((PHANDLE_GRANT_ACCESS_DATA)pIoBuffer);
		else
			status = STATUS_INFO_LENGTH_MISMATCH;
	}
	break;
	case IOCTL_POWER_READ_MEMORY_C:
	{
		if (ulInputSize >= sizeof(READ_WRITE_MEMORY_DATA) && pIoBuffer)
			status = PowerReadVirtualMemoryC((PREAD_WRITE_MEMORY_DATA)pIoBuffer);
		else
			status = STATUS_INFO_LENGTH_MISMATCH;
	}
	break;
	case IOCTL_POWER_WRITE_MEMORY_C:
	{
		if (ulInputSize >= sizeof(READ_WRITE_MEMORY_DATA) && pIoBuffer)
			status = PowerWriteVirtualMemoryC((PREAD_WRITE_MEMORY_DATA)pIoBuffer);
		else
			status = STATUS_INFO_LENGTH_MISMATCH;
	}
	break;
	case IOCTL_POWER_WRITE_MEMORY_B:
	{
		if (ulInputSize >= sizeof(READ_WRITE_MEMORY_DATA) && pIoBuffer)
			status = PowerWriteVirtualMemoryB((PREAD_WRITE_MEMORY_DATA)pIoBuffer);
		else
			status = STATUS_INFO_LENGTH_MISMATCH;
	}
	break;
	default:
		LOGDEBUG(ulIoControlCode, "Unknown IRP_MJ_DEVICE_CONTROL");
		status = STATUS_INVALID_PARAMETER;
		break;
	}

	// retn complete bytes
	if (status == STATUS_SUCCESS)
		pIrp->IoStatus.Information = ulOutputSize;
	else
		pIrp->IoStatus.Information = 0;
	// retn status
	pIrp->IoStatus.Status = status;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	return status;
}