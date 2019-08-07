#include "main.h"


BOOL PowerReadMemory(__in ULONG ulPid, __in PVOID pStartAddress, __in ULONG ulSize, __out PVOID pSaveAddress)
{
	READ_WRITE_MEMORY_DATA pData = { 0 };
	pData.ulPid = ulPid;
	pData.pAddress = pStartAddress;
	pData.ulSize = ulSize;
	pData.pBuffer = pSaveAddress;

	DWORD dwRealRetByte = 0;
	if (IoCtrlDriver(IOCTL_POWER_READ_MEMORY, &pData, sizeof(pData), &pData, sizeof(pData), &dwRealRetByte))
		return TRUE;
	else
		return FALSE;
}

BOOL PowerWriteMemory(__in ULONG ulPid, __in PVOID pStartAddress, __in ULONG ulSize, __out PVOID pWriteBuffer)
{
	READ_WRITE_MEMORY_DATA pData = { 0 };
	pData.ulPid = ulPid;
	pData.pAddress = pStartAddress;
	pData.ulSize = ulSize;
	pData.pBuffer = pWriteBuffer;

	DWORD dwRealRetByte = 0;
	if (IoCtrlDriver(IOCTL_POWER_WRITE_MEMORY, &pData, sizeof(pData), &pData, sizeof(pData), &dwRealRetByte))
		return TRUE;
	else
		return FALSE;
}

// ͨ����������ȡPID
DWORD GetProcessIdByName(__in const PTCHAR wsProcessName)
{
	// �������̿���
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	// ��������
	PROCESSENTRY32 stProcessEntry = PROCESSENTRY32{ sizeof(PROCESSENTRY32) };

	if (Process32First(hSnapshot, &stProcessEntry))
	{
		do
		{
			if (!_tcsicmp(wsProcessName, stProcessEntry.szExeFile))
			{
				CloseHandle(hSnapshot);
				return stProcessEntry.th32ProcessID;
			}
		} while (Process32Next(hSnapshot, &stProcessEntry));
	}
	CloseHandle(hSnapshot);

	// û�ҵ�
	return NULL;
}

int main()
{
	PRINTMSG(POWER_SUCCESS, "PowerReadWrite");

	// ��������
	if (!LoadDriver())
	{
		UnloadDriver();
	}
	
	// ���豸
	if (OpenDevice((PTCHAR)DEVICE_LINKNAME))
	{
		PRINTMSG(POWER_SUCCESS, "OpenDevice Success");
	}
	

	// ���ṹ��
	HANDLE hProcess = NULL;
	SET_PROCESS_DATA stSetProcessData = { 0 };
	stSetProcessData.ulPid = GetProcessIdByName((const PTCHAR)_T("calc.exe"));
	stSetProcessData.ulAccess = PROCESS_QUERY_LIMITED_INFORMATION;
	stSetProcessData.pHandleProcess = &hProcess;

	PRINTMSG(stSetProcessData.ulPid, "SetProcess Pid");
	PRINTMSG(hProcess, "SetProcess Handle");
	PRINTMSG(stSetProcessData.ulAccess, "SetProcess Access");

	////��������һ�ֶ�д��ʽ���õĻ�ֱ��ȥ��ע��
	//ULONG ulBuffer;
	//PowerReadMemory(GetProcessIdByName((const PTCHAR)_T("calc.exe")), (PVOID)0xFF740000, 4, &ulBuffer);
	//PRINTMSG(ulBuffer, "PowerReadMemory Buffer");
	//PowerWriteMemory(GetProcessIdByName((const PTCHAR)_T("calc.exe")), (PVOID)0xFF7B3000, 4, &ulBuffer);
	//PowerReadMemory(GetProcessIdByName((const PTCHAR)_T("calc.exe")), (PVOID)0xFF7B3000, 4, &ulBuffer);
	//PRINTMSG(ulBuffer, "PowerWriteMemory Buffer");
	//PRINTMSG(POWER_SUCCESS, "Please press any key UnloadDriver");
	//getchar();
	//CloseDevice();
	//UnloadDriver();
	//getchar();
	//return 0;

	// ��������
	DWORD dwRealRetByte = 0;
	IoCtrlDriver(IOCTL_POWER_SET_PROCESS, &stSetProcessData, sizeof(stSetProcessData), &stSetProcessData, sizeof(stSetProcessData), &dwRealRetByte);

	PRINTMSG(hProcess, "OpenProcess Handle");

	// �򿪾��Ȩ�޲����Ļ���������һ��Ȩ��
	HANDLE_GRANT_ACCESS_DATA stHandleAccessData = { 0 };
	stHandleAccessData.hProcess = hProcess;
	stHandleAccessData.ulPid = GetCurrentProcessId();
	stHandleAccessData.ulAccess = PROCESS_ALL_ACCESS;
	IoCtrlDriver(IOCTL_POWER_GRANT_ACCESS, &stHandleAccessData, sizeof(stHandleAccessData), &stHandleAccessData, sizeof(stHandleAccessData), &dwRealRetByte);


	HANDLE handle = hProcess;

	ULONG return_len;
	PEB   process_peb;
	PROCESS_BASIC_INFORMATION process_info;
	RTL_USER_PROCESS_PARAMETERS process_parameters;
	WCHAR buffer[512];
	typfnNtQueryInformationProcess pfnNtQueryInformationProcess = (typfnNtQueryInformationProcess)GetProcAddress(LoadLibrary(_T("ntdll.dll")),"NtQueryInformationProcess");

	if (pfnNtQueryInformationProcess(handle, ProcessBasicInformation, &process_info, sizeof(process_info), &return_len) < 0)
	{
		PRINTMSG(POWER_UNSUCCESSFUL, "NtQueryInformationProcess failed");
	}

	if (!ReadProcessMemory(handle, process_info.PebBaseAddress, &process_peb, sizeof(process_peb), nullptr) ||
		!ReadProcessMemory(handle, process_peb.ProcessParameters, &process_parameters, sizeof(process_parameters), nullptr) ||
		!ReadProcessMemory(handle, process_parameters.CommandLine.Buffer, buffer, process_parameters.CommandLine.Length, nullptr))
	{
		PRINTMSG(POWER_UNSUCCESSFUL, "ReadProcessMemory failed");
	}
	else
	{
		PRINTMSG(POWER_SUCCESS, "ReadProcessMemory Success");
	}

	printf("[Power]->[0x%p]->CommandLine: %ws", POWER_SUCCESS, buffer);
	printf("\n");


	PRINTMSG(POWER_SUCCESS, "Please press any key UnloadDriver");
	getchar();
	CloseHandle(handle);
	CloseDevice();
	UnloadDriver();
	getchar();
	return 0;
}