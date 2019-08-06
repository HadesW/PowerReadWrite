#include "main.h"




// 通过进程名获取PID
DWORD GetProcessIdByName(__in const PTCHAR wsProcessName)
{
	// 创建进程快照
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	// 遍历进程
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

	// 没找到
	return NULL;
}

int main()
{
	PRINTMSG(POWER_SUCCESS, "PowerReadWrite");

	// 加载驱动
	if (!LoadDriver())
	{
		UnloadDriver();
	}
	
	// 打开设备
	if (OpenDevice((PTCHAR)DEVICE_LINKNAME))
	{
		PRINTMSG(POWER_SUCCESS, "OpenDevice Success");
	}
	

	// 填充结构体
	HANDLE hProcess = NULL;
	SET_PROCESS_DATA stSetProcessData = { 0 };
	stSetProcessData.ulPid = GetProcessIdByName((const PTCHAR)_T("crossfire.exe"));
	stSetProcessData.ulAccess = PROCESS_QUERY_LIMITED_INFORMATION;
	stSetProcessData.pHandleProcess = &hProcess;

	PRINTMSG(stSetProcessData.ulPid, "SetProcess Pid");
	PRINTMSG(hProcess, "SetProcess Handle");
	PRINTMSG(stSetProcessData.ulAccess, "SetProcess Access");

	// 发送数据
	DWORD dwRealRetByte = 0;
	IoCtrlDriver(IOCTL_POWER_SET_PROCESS, &stSetProcessData, sizeof(stSetProcessData), &stSetProcessData, sizeof(stSetProcessData), &dwRealRetByte);

	PRINTMSG(hProcess, "OpenProcess Handle");

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
	};

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