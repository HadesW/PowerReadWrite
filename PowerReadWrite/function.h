#pragma once
#include "main.h"

typedef struct _OPENPROCESS_DATA
{
	ULONG ulPid;
	ACCESS_MASK ulAccess;
	PHANDLE pHandleProcess;
}OPENPROCESS_DATA,*POPENPROCESS_DATA;


NTSTATUS PowerOpenProcess(POPENPROCESS_DATA pData);