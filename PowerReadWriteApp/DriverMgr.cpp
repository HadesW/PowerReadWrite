#include "DriverMgr.h"

// 驱动文件
TCHAR g_wsSysFilePath[MAX_PATH] = { 0 };
// 服务名
#define SERVICE_NAME  _T("PRWService")

// 服务管理器句柄
SC_HANDLE g_hSCManager = NULL;
SC_HANDLE g_hService = NULL;
HANDLE g_hDevice = INVALID_HANDLE_VALUE;

BOOL Install()
{
	TCHAR* wsServiceName = (TCHAR*)SERVICE_NAME;
	TCHAR*wsDispayName = wsServiceName;
	TCHAR*wsSysFilePath = g_wsSysFilePath;
	// 打开服务管理器
	g_hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (!g_hSCManager) { return FALSE; }

	// 创建服务
	g_hService = CreateService(
		g_hSCManager,						// SCM句柄
		wsServiceName,						// 驱动服务名称(驱动程序的在注册表中的名字)
		wsDispayName,						// 驱动服务显示名称(注册表驱动程序的DisplayName值)
		SERVICE_ALL_ACCESS,				// 访问权限(所有访问权限)
		SERVICE_KERNEL_DRIVER,			// 服务类型(驱动程序)
		SERVICE_DEMAND_START,		// 启动方式(需要时启动,注册表驱动程序的Start值)
		SERVICE_ERROR_NORMAL,		// 错误控制(忽略,注册表驱动程序的ErrorControl值)
		wsSysFilePath,							// 服务的二进制文件路径(驱动程序文件路径, 注册表驱动程序的ImagePath值)
		NULL,										//加载组命令
		NULL,										//TagId(指向一个加载顺序的标签值)
		NULL,										//依存关系
		NULL,										//服务启动名
		NULL);										//密码
	// 创建失败
	if (g_hService==NULL)
	{
		// 错误码是服务已经存在
		if (GetLastError()==ERROR_SERVICE_EXISTS)
		{
			// 直接打开
			g_hService= OpenService(g_hSCManager, wsServiceName, SERVICE_ALL_ACCESS);
			// 打开还是失败,就是真的失败了
			if (g_hService==NULL)
			{
				CloseServiceHandle(g_hSCManager);
				g_hSCManager = NULL;
				return FALSE;
			}
		}
		// 错误码是其他的，也是真的失败
		else
		{
			CloseServiceHandle(g_hSCManager);
			g_hSCManager = NULL;
			return FALSE;
		}
	}

	// 安装成功
	CloseServiceHandle(g_hSCManager);
	g_hSCManager = NULL;
	return TRUE;
}

BOOL Start()
{
	// 服务已经打开了
	if (g_hService)
	{
		if (!StartService(g_hService, NULL, NULL))
			return FALSE;
		else
			return TRUE;//开启服务成功
	}
	return FALSE;
}


BOOL Stop()
{
	// 打开服务管理器
	if (g_hSCManager==NULL)
	{
		g_hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
		if (!g_hSCManager) { return FALSE; }
	}
	// 如果服务没打开，尝试打开
	if (!g_hService)
	{
		g_hService = OpenService(g_hSCManager, SERVICE_NAME, SERVICE_ALL_ACCESS);
		// 尝试打开失败
		if (!g_hService)
		{
			CloseServiceHandle(g_hSCManager);
			g_hSCManager = NULL;
			return FALSE;
		}
	}
	// 如果服务已经打开了直接停止服务
	SERVICE_STATUS ss;
	if (!ControlService(g_hService, SERVICE_CONTROL_STOP, &ss))
	{
		CloseServiceHandle(g_hSCManager);
		g_hSCManager = NULL;
		return FALSE;
	}
	// 成功
	CloseServiceHandle(g_hSCManager);
	g_hSCManager = NULL;
	return TRUE;
}


BOOL Uninstall()
{
	// 打开服务管理器
	if (g_hSCManager == NULL)
	{
		g_hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
		if (!g_hSCManager) { return FALSE; }
	}
	// 如果服务没打开，尝试打开
	if (!g_hService)
	{
		g_hService = OpenService(g_hSCManager, SERVICE_NAME, SERVICE_ALL_ACCESS);
		// 尝试打开失败
		if (!g_hService)
		{
			CloseServiceHandle(g_hSCManager);
			g_hSCManager = NULL;
			return FALSE;
		}
	}
	// 如果服务已经打开了直接删除服务
	if (!DeleteService(g_hService))
	{
		CloseServiceHandle(g_hSCManager);
		g_hSCManager = NULL;
		return FALSE;
	}

	CloseServiceHandle(g_hSCManager);
	g_hSCManager = NULL;
	return TRUE;
}



BOOL UnloadDriver()
{
	// 不管驱动是否存在都进行停止删除操作
	// 停止运行服务
	if (!Stop())
	{
		PRINTMSG(GetLastError(), "StopDriver LastError");
	}

	// 卸载服务
	if (!Uninstall())
	{
		PRINTMSG(GetLastError(), "UninstallDriver LastError");
	}

	PRINTMSG(POWER_SUCCESS, "UnloadDriver Success");
	return TRUE;
}



BOOL LoadDriver()
{
	// 先获取一下驱动文件路径
	GetSysFullPath(_T("PowerReadWrite.sys"));

	// 安装服务
	if (Install())
	{
		// 开启服务
		if (Start())
		{
			PRINTMSG(POWER_SUCCESS, "LoadDriver Success");
		}
		else
		{
			PRINTMSG(GetLastError(), "StartDriver LastError");
		}
	}
	else
	{
		PRINTMSG(GetLastError(), "InstallDriver LastError");
	}
	return TRUE;
}


BOOL OpenDevice(PTCHAR wsDevLinkName)
{
	if (g_hDevice != INVALID_HANDLE_VALUE)
		return TRUE;
	g_hDevice = CreateFile(wsDevLinkName, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (g_hDevice != INVALID_HANDLE_VALUE)
		return TRUE;
	else
		return FALSE;
}

BOOL CloseDevice()
{
	if (g_hDevice == INVALID_HANDLE_VALUE)
		return FALSE;
	else
		CloseHandle(g_hDevice);
	g_hDevice = INVALID_HANDLE_VALUE;
	return TRUE;
}

DWORD CTL_CODE_GEN(DWORD lngFunction)
{
	return (ULONG)CTL_CODE(FILE_DEVICE_UNKNOWN, lngFunction, METHOD_BUFFERED, FILE_ANY_ACCESS);
}
// 使用控制码控制驱动
BOOL IoCtrlDriver(DWORD dwIoCode, PVOID pInBuffer, DWORD dwInBufferLen, PVOID pOutBuffer, DWORD dwOutBufferLen, DWORD *RealRetBytes)
{
	DWORD dwRetByte;
	BOOL bRet = DeviceIoControl(g_hDevice, CTL_CODE_GEN(dwIoCode), pInBuffer, dwInBufferLen, pOutBuffer, dwOutBufferLen, &dwRetByte, NULL);
	if (RealRetBytes)
		*RealRetBytes = dwRetByte;
	return bRet;
}







// 获取驱动文件全路径
VOID GetSysFullPath(PTCHAR wsSysFileName)
{
	GetModuleFileName(0, g_wsSysFilePath, MAX_PATH);
	for (size_t i = _tcslen(g_wsSysFilePath) - 1; i > 0; i--)
	{
		if (g_wsSysFilePath[i] == '\\')
		{
			g_wsSysFilePath[i + 1] = '\0';
			break;
		}
	}
	_tcscat_s(g_wsSysFilePath, wsSysFileName);
	return;
}