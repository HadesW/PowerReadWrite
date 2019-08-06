#pragma once
#include "windows.h"
#include <stdio.h>
#include "DriverMgr.h"
#include <TlHelp32.h>
#include <winternl.h>

#define POWER_SUCCESS 1
#define POWER_UNSUCCESSFUL 0

#define PRINTMSG(var,message) {printf("[Power]->[0x%p]->%s \n",var,message);}

//#define DEVICE_LINKNAME L"\\??\\PowerLinkName"
//#define IOCTL_POWER_OPENPROCESS 0x801


typedef struct _SET_PROCESS_DATA
{
	ULONG ulPid;
	ACCESS_MASK ulAccess;
	PHANDLE pHandleProcess;
}SET_PROCESS_DATA,*PSET_PROCESS_DATA;

typedef struct _HANDLE_GRANT_ACCESS_DATA
{
	ULONG ulPid;
	ACCESS_MASK ulAccess;
	HANDLE hProcess;
}HANDLE_GRANT_ACCESS_DATA, *PHANDLE_GRANT_ACCESS_DATA;

//__kernel_entry NTSTATUS NtQueryInformationProcess(
//	IN HANDLE           ProcessHandle,
//	IN PROCESSINFOCLASS ProcessInformationClass,
//	OUT PVOID           ProcessInformation,
//	IN ULONG            ProcessInformationLength,
//	OUT PULONG          ReturnLength
//);
typedef NTSTATUS(NTAPI *typfnNtQueryInformationProcess)(
	IN  HANDLE ProcessHandle,
	IN  PROCESSINFOCLASS ProcessInformationClass,
	OUT PVOID ProcessInformation,
	IN  ULONG ProcessInformationLength,
	OUT PULONG ReturnLength OPTIONAL
	);