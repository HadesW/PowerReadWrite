#include "function.h"


OPENPROCESS_DATA g_stOpenProcessData = { 0 };
HANDLE g_hProcess = NULL;

NTSTATUS PowerOpenProcess(POPENPROCESS_DATA pData)
{
	LOGDEBUG(PowerOpenProcess, "PowerOpenProcess Called");

	//INT3BREAKPOINT;

	NTSTATUS status = STATUS_SUCCESS;
	PEPROCESS pEprocess;
	HANDLE hProcess;

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
		PROCESS_ALL_ACCESS,
		*PsProcessType,
		KernelMode,
		&hProcess
	);
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