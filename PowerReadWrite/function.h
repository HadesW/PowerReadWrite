#pragma once
#include "main.h"

#define EX_ADDITIONAL_INFO_SIGNATURE (ULONG_PTR)(-2)
#define ExpIsValidObjectEntry(Entry) \
    ( (Entry != NULL) && (Entry->LowValue != 0) && (Entry->HighValue != EX_ADDITIONAL_INFO_SIGNATURE) )

typedef enum _ENUM_WINDOWS_VERSION
{
	WINDOWS_7 = 0x0610,
	WINDOWS_7_SP1 = 0x0611,
	WINDOWS_8 = 0x0620,
	WINDOWS_8_1 = 0x0630,
	WINDOWS_10 = 0x0A00,
	WINDOWS_10_RS1 = 0x0A01, // Anniversary update
	WINDOWS_10_RS2 = 0x0A02, // Creators update
	WINDOWS_10_RS3 = 0x0A03, // Fall creators update
	WINDOWS_10_RS4 = 0x0A04, // Spring creators update
	WINDOWS_10_RS5 = 0x0A05, // October 2018 update
} ENUM_WINDOWS_VERSION;


typedef struct _OPENPROCESS_DATA
{
	ULONG ulPid;
	ACCESS_MASK ulAccess;
	PHANDLE pHandleProcess;//´«³ö
}OPENPROCESS_DATA,*POPENPROCESS_DATA;

typedef struct _HANDLE_GRANT_ACCESS_DATA
{
	ULONG ulPid;
	ACCESS_MASK ulAccess;
	HANDLE hProcess;
}HANDLE_GRANT_ACCESS_DATA, *PHANDLE_GRANT_ACCESS_DATA;

typedef struct _READ_WRITE_MEMORY_DATA
{
	ULONG ulPid;
	PVOID pAddress;
	ULONG ulSize;
	PVOID pBuffer;
}READ_WRITE_MEMORY_DATA,*PREAD_WRITE_MEMORY_DATA;


typedef union _EXHANDLE
{
	struct
	{
		int TagBits : 2;
		int Index : 30;
	} u;
	void * GenericHandleOverlay;
	ULONG_PTR Value;
} EXHANDLE, *PEXHANDLE;

typedef struct _HANDLE_TABLE_ENTRY // Size=16
{
	union
	{
		ULONG_PTR VolatileLowValue; // Size=8 Offset=0
		ULONG_PTR LowValue; // Size=8 Offset=0
		struct _HANDLE_TABLE_ENTRY_INFO * InfoTable; // Size=8 Offset=0
		struct
		{
			ULONG_PTR Unlocked : 1; // Size=8 Offset=0 BitOffset=0 BitCount=1
			ULONG_PTR RefCnt : 16; // Size=8 Offset=0 BitOffset=1 BitCount=16
			ULONG_PTR Attributes : 3; // Size=8 Offset=0 BitOffset=17 BitCount=3
			ULONG_PTR ObjectPointerBits : 44; // Size=8 Offset=0 BitOffset=20 BitCount=44
		};
	};
	union
	{
		ULONG_PTR HighValue; // Size=8 Offset=8
		struct _HANDLE_TABLE_ENTRY * NextFreeHandleEntry; // Size=8 Offset=8
		union _EXHANDLE LeafHandleValue; // Size=8 Offset=8
		struct
		{
			ULONG GrantedAccessBits : 25; // Size=4 Offset=8 BitOffset=0 BitCount=25
			ULONG NoRightsUpgrade : 1; // Size=4 Offset=8 BitOffset=25 BitCount=1
			ULONG Spare : 6; // Size=4 Offset=8 BitOffset=26 BitCount=6
		};
	};
	ULONG TypeInfo; // Size=4 Offset=12
} HANDLE_TABLE_ENTRY, *PHANDLE_TABLE_ENTRY;

typedef struct _HANDLE_TABLE
{
	ULONG_PTR TableCode;
	struct _EPROCESS *QuotaProcess;
	HANDLE UniqueProcessId;
	void* HandleLock;
	struct _LIST_ENTRY HandleTableList;
	EX_PUSH_LOCK HandleContentionEvent;
	struct _HANDLE_TRACE_DEBUG_INFO *DebugInfo;
	int ExtraInfoPages;
	ULONG Flags;
	ULONG FirstFreeHandle;
	struct _HANDLE_TABLE_ENTRY *LastFreeHandleEntry;
	ULONG HandleCount;
	ULONG NextHandleNeedingPool;
	// More fields here...
} HANDLE_TABLE, *PHANDLE_TABLE;

typedef BOOLEAN(*typfnEX_ENUMERATE_HANDLE_ROUTINE_Win7)(
	IN PHANDLE_TABLE_ENTRY HandleTableEntry,
	IN HANDLE Handle,
	IN PVOID EnumParameter
	);
typedef BOOLEAN(*typfnEX_ENUMERATE_HANDLE_ROUTINE_Win10)(
	IN PHANDLE_TABLE HandleTable,
	IN PHANDLE_TABLE_ENTRY HandleTableEntry,
	IN HANDLE Handle,
	IN PVOID EnumParameter
	);

//typedef BOOLEAN(*EX_ENUMERATE_HANDLE_ROUTINE)(
//IN PHANDLE_TABLE HandleTable,
//	IN PHANDLE_TABLE_ENTRY HandleTableEntry,
//	IN HANDLE Handle,
//	IN PVOID EnumParameter
//	);
//
//NTKERNELAPI
//BOOLEAN
//ExEnumHandleTable(
//	IN PHANDLE_TABLE HandleTable,
//	IN EX_ENUMERATE_HANDLE_ROUTINE EnumHandleProcedure,
//	IN PVOID EnumParameter,
//	OUT PHANDLE Handle
//);
typedef BOOLEAN(*typfnExEnumHandleTableWin7)(
		IN PHANDLE_TABLE HandleTable,
		IN typfnEX_ENUMERATE_HANDLE_ROUTINE_Win7 EnumHandleProcedure,
		IN PVOID EnumParameter,
		OUT PHANDLE Handle
	);
typedef BOOLEAN(*typfnExEnumHandleTableWin10)(
	IN PHANDLE_TABLE HandleTable,
	IN typfnEX_ENUMERATE_HANDLE_ROUTINE_Win10 EnumHandleProcedure,
	IN PVOID EnumParameter,
	OUT PHANDLE Handle
	);

NTKERNELAPI
VOID
FASTCALL
ExfUnblockPushLock(
	IN OUT PEX_PUSH_LOCK PushLock,
	IN OUT PVOID WaitBlock
);

NTSTATUS
MmCopyVirtualMemory(
	IN PEPROCESS FromProcess,
	IN CONST VOID *FromAddress,
	IN PEPROCESS ToProcess,
	OUT PVOID ToAddress,
	IN SIZE_T BufferSize,
	IN KPROCESSOR_MODE PreviousMode,
	OUT PSIZE_T NumberOfBytesCopied
);

NTSTATUS PowerOpenProcess(__in POPENPROCESS_DATA pData);
NTSTATUS PowerGrantAccess(__in PHANDLE_GRANT_ACCESS_DATA pData);
NTSTATUS PowerReadVirtualMemory(__in PREAD_WRITE_MEMORY_DATA pData);
NTSTATUS PowerWriteVirtualMemory(__in PREAD_WRITE_MEMORY_DATA pData);