#include "function.h"


OPENPROCESS_DATA g_stOpenProcessData = { 0 };
HANDLE g_hProcess = NULL;

NTSTATUS PowerOpenProcess(__in POPENPROCESS_DATA pData)
{
	LOGDEBUG(PowerOpenProcess, "PowerOpenProcess Called");

	//INT3BREAKPOINT;

	NTSTATUS status = STATUS_SUCCESS;
	PEPROCESS pEprocess=NULL;
	HANDLE hProcess=NULL;

	// check address
	__try
	{
		ProbeForWrite(pData->pHandleProcess, sizeof(HANDLE), sizeof(HANDLE));
		ProbeForRead(pData->ulPid, sizeof(ULONG), sizeof(ULONG));
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		return GetExceptionCode();
	}

	// lookup EPROCESS
	status = PsLookupProcessByProcessId(pData->ulPid, &pEprocess);
	if (!NT_SUCCESS(status))
		return status;
	// Always open in KernelMode to skip ordinary access checks.
	status = ObOpenObjectByPointer(
		pEprocess,
		0,
		NULL,
		pData->ulAccess,//PROCESS_ALL_ACCESS
		*PsProcessType,
		KernelMode,
		&hProcess
	);
	if (pEprocess)
		ObDereferenceObject(pEprocess);
	
	// output
	*(pData->pHandleProcess) = hProcess;

	// save global variable
	g_hProcess = hProcess;
	g_stOpenProcessData.pHandleProcess = &g_hProcess;
	g_stOpenProcessData.ulPid = pData->ulPid;
	g_stOpenProcessData.ulAccess = pData->ulAccess;

	return status;
}


BOOLEAN HandleCallbackWin7(
	IN PHANDLE_TABLE_ENTRY HandleTableEntry,
	IN HANDLE Handle,
	IN PVOID EnumParameter
)
{
	INT3BREAKPOINT;

	BOOLEAN bResult = FALSE;
	ASSERT(EnumParameter);

	if (EnumParameter != NULL)
	{
		PHANDLE_GRANT_ACCESS_DATA pAccess = (PHANDLE_GRANT_ACCESS_DATA)EnumParameter;
		if (Handle == (HANDLE)pAccess->hProcess)
		{
			if (ExpIsValidObjectEntry(HandleTableEntry))
			{
				// Update access
				HandleTableEntry->GrantedAccessBits = pAccess->ulAccess;
				bResult = TRUE;
			}
			else
			{
				LOGDEBUG(pAccess->hProcess,"HandleCallback:handle is invalid")
			}
		}
	}

	return bResult;
}

BOOLEAN HandleCallbackWin10(
	IN PHANDLE_TABLE HandleTable,
	IN PHANDLE_TABLE_ENTRY HandleTableEntry,
	IN HANDLE Handle,
	IN PVOID EnumParameter
)
{
	BOOLEAN bResult = FALSE;
	ASSERT(EnumParameter);

	if (EnumParameter != NULL)
	{
		PHANDLE_GRANT_ACCESS_DATA pAccess = (PHANDLE_GRANT_ACCESS_DATA)EnumParameter;
		if (Handle == (HANDLE)pAccess->hProcess)
		{
			if (ExpIsValidObjectEntry(HandleTableEntry))
			{
				// Update access
				HandleTableEntry->GrantedAccessBits = pAccess->ulAccess;
				bResult = TRUE;
			}
			else
			{
				LOGDEBUG(pAccess->hProcess, "HandleCallback:handle is invalid")
			}
		}
	}
	// Release implicit locks
	_InterlockedExchangeAdd8((char*)&HandleTableEntry->VolatileLowValue, 1);  // Set Unlocked flag to 1
	if (HandleTable != NULL && HandleTable->HandleContentionEvent)
		ExfUnblockPushLock(&HandleTable->HandleContentionEvent, NULL);

	return bResult;
}


NTSTATUS PowerGrantAccess(__in PHANDLE_GRANT_ACCESS_DATA pData)
{
	LOGDEBUG(PowerGrantAccess, "PowerGrantAccess Called");

	INT3BREAKPOINT;

	NTSTATUS status = STATUS_SUCCESS;
	PEPROCESS pEprocess = NULL;
	ULONG ulObjectTableOffset = 0x0;

	// lookup EPROCESS
	status = PsLookupProcessByProcessId(pData->ulPid, &pEprocess);
	if (!NT_SUCCESS(status))
		return status;

	// Get kernel version
	RTL_OSVERSIONINFOEXW stOSVersionInfo = { 0 };
	stOSVersionInfo.dwOSVersionInfoSize = sizeof(stOSVersionInfo);
	status = RtlGetVersion((PRTL_OSVERSIONINFOW)&stOSVersionInfo);
	if (!NT_SUCCESS(status))
		return status;

	UNICODE_STRING usFunctionName;
	RtlInitUnicodeString(&usFunctionName, L"ExEnumHandleTable");
	PVOID pfnExEnumHandleTable=MmGetSystemRoutineAddress(&usFunctionName);
	if (!pfnExEnumHandleTable)
		return STATUS_NOT_FOUND;

	ULONG ulWindowsVersion = (stOSVersionInfo.dwMajorVersion << 8) | (stOSVersionInfo.dwMinorVersion << 4) | stOSVersionInfo.wServicePackMajor;
	switch (ulWindowsVersion)
	{
	case WINDOWS_7:
	case WINDOWS_7_SP1:
	{
		ulObjectTableOffset = 0x200;
		PHANDLE_TABLE pTable = *(PHANDLE_TABLE*)((PUCHAR)pEprocess + ulObjectTableOffset);
		typfnExEnumHandleTableWin7 pfnExEnumHandleTableWin7 = pfnExEnumHandleTable;
		BOOLEAN bFound = pfnExEnumHandleTableWin7(pTable,&HandleCallbackWin7, pData, NULL);
		if (bFound == FALSE)
			status = STATUS_NOT_FOUND;
	}
		break;
	case WINDOWS_10:
	case WINDOWS_10_RS1:
	case WINDOWS_10_RS2:
	case WINDOWS_10_RS3:	
	case WINDOWS_10_RS4:
	{
		ulObjectTableOffset = 0x418;
		PHANDLE_TABLE pTable = *(PHANDLE_TABLE*)((PUCHAR)pEprocess + ulObjectTableOffset);
		typfnExEnumHandleTableWin10 pfnExEnumHandleTableWin10 = pfnExEnumHandleTable;
		BOOLEAN bFound = pfnExEnumHandleTableWin10(pTable, &HandleCallbackWin10, pData, NULL);
		if (bFound == FALSE)
			status = STATUS_NOT_FOUND;
	}
		break;
	default:
	{
		LOGDEBUG(stOSVersionInfo.dwBuildNumber, "Unsupported System Version");
		status = STATUS_NOT_SUPPORTED;
	}
		break;
	}

	if (pEprocess)
		ObDereferenceObject(pEprocess);

	return status;
}

NTSTATUS PowerReadVirtualMemory(__in PREAD_WRITE_MEMORY_DATA pData)
{
	NTSTATUS status=STATUS_UNSUCCESSFUL;
	SIZE_T NumberOfBytes=NULL;
	PEPROCESS pEprocess=NULL;

	// check address
	__try
	{
		ProbeForWrite(pData->pBuffer, pData->ulSize, sizeof(CHAR));
		ProbeForRead(pData->ulPid, sizeof(ULONG), sizeof(ULONG));
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		return GetExceptionCode();
	}
	// read
	status = PsLookupProcessByProcessId(pData->ulPid, &pEprocess);
	if (NT_SUCCESS(status))
	{
		status = MmCopyVirtualMemory(pEprocess, pData->pAddress, PsGetCurrentProcess(), pData->pBuffer, pData->ulSize, KernelMode, &NumberOfBytes);

		if (pEprocess)
			ObDereferenceObject(pEprocess);
	}

	return status;
}

NTSTATUS PowerWriteVirtualMemory(__in PREAD_WRITE_MEMORY_DATA pData)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	SIZE_T NumberOfBytes = NULL;
	PEPROCESS pEprocess = NULL;

	// check address
	__try
	{
		ProbeForRead(pData->pAddress, pData->ulSize, sizeof(CHAR));
		ProbeForRead(pData->ulPid, sizeof(ULONG), sizeof(ULONG));
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		return GetExceptionCode();
	}
	// write
	status = PsLookupProcessByProcessId(pData->ulPid, &pEprocess);
	if (NT_SUCCESS(status))
	{
		status = MmCopyVirtualMemory(PsGetCurrentProcess(), pData->pBuffer, pEprocess,pData->pAddress,pData->ulSize,KernelMode,&NumberOfBytes);

		if (pEprocess)
			ObDereferenceObject(pEprocess);
	}

	return status;
}