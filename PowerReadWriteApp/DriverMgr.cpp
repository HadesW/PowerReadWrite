#include "DriverMgr.h"

// �����ļ�
TCHAR g_wsSysFilePath[MAX_PATH] = { 0 };
// ������
#define SERVICE_NAME  _T("PRWService")

// ������������
SC_HANDLE g_hSCManager = NULL;
SC_HANDLE g_hService = NULL;
HANDLE g_hDevice = INVALID_HANDLE_VALUE;

BOOL Install()
{
	TCHAR* wsServiceName = (TCHAR*)SERVICE_NAME;
	TCHAR*wsDispayName = wsServiceName;
	TCHAR*wsSysFilePath = g_wsSysFilePath;
	// �򿪷��������
	g_hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (!g_hSCManager) { return FALSE; }

	// ��������
	g_hService = CreateService(
		g_hSCManager,						// SCM���
		wsServiceName,						// ������������(�����������ע����е�����)
		wsDispayName,						// ����������ʾ����(ע������������DisplayNameֵ)
		SERVICE_ALL_ACCESS,				// ����Ȩ��(���з���Ȩ��)
		SERVICE_KERNEL_DRIVER,			// ��������(��������)
		SERVICE_DEMAND_START,		// ������ʽ(��Ҫʱ����,ע������������Startֵ)
		SERVICE_ERROR_NORMAL,		// �������(����,ע������������ErrorControlֵ)
		wsSysFilePath,							// ����Ķ������ļ�·��(���������ļ�·��, ע������������ImagePathֵ)
		NULL,										//����������
		NULL,										//TagId(ָ��һ������˳��ı�ǩֵ)
		NULL,										//�����ϵ
		NULL,										//����������
		NULL);										//����
	// ����ʧ��
	if (g_hService==NULL)
	{
		// �������Ƿ����Ѿ�����
		if (GetLastError()==ERROR_SERVICE_EXISTS)
		{
			// ֱ�Ӵ�
			g_hService= OpenService(g_hSCManager, wsServiceName, SERVICE_ALL_ACCESS);
			// �򿪻���ʧ��,�������ʧ����
			if (g_hService==NULL)
			{
				CloseServiceHandle(g_hSCManager);
				g_hSCManager = NULL;
				return FALSE;
			}
		}
		// �������������ģ�Ҳ�����ʧ��
		else
		{
			CloseServiceHandle(g_hSCManager);
			g_hSCManager = NULL;
			return FALSE;
		}
	}

	// ��װ�ɹ�
	CloseServiceHandle(g_hSCManager);
	g_hSCManager = NULL;
	return TRUE;
}

BOOL Start()
{
	// �����Ѿ�����
	if (g_hService)
	{
		if (!StartService(g_hService, NULL, NULL))
			return FALSE;
		else
			return TRUE;//��������ɹ�
	}
	return FALSE;
}


BOOL Stop()
{
	// �򿪷��������
	if (g_hSCManager==NULL)
	{
		g_hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
		if (!g_hSCManager) { return FALSE; }
	}
	// �������û�򿪣����Դ�
	if (!g_hService)
	{
		g_hService = OpenService(g_hSCManager, SERVICE_NAME, SERVICE_ALL_ACCESS);
		// ���Դ�ʧ��
		if (!g_hService)
		{
			CloseServiceHandle(g_hSCManager);
			g_hSCManager = NULL;
			return FALSE;
		}
	}
	// ��������Ѿ�����ֱ��ֹͣ����
	SERVICE_STATUS ss;
	if (!ControlService(g_hService, SERVICE_CONTROL_STOP, &ss))
	{
		CloseServiceHandle(g_hSCManager);
		g_hSCManager = NULL;
		return FALSE;
	}
	// �ɹ�
	CloseServiceHandle(g_hSCManager);
	g_hSCManager = NULL;
	return TRUE;
}


BOOL Uninstall()
{
	// �򿪷��������
	if (g_hSCManager == NULL)
	{
		g_hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
		if (!g_hSCManager) { return FALSE; }
	}
	// �������û�򿪣����Դ�
	if (!g_hService)
	{
		g_hService = OpenService(g_hSCManager, SERVICE_NAME, SERVICE_ALL_ACCESS);
		// ���Դ�ʧ��
		if (!g_hService)
		{
			CloseServiceHandle(g_hSCManager);
			g_hSCManager = NULL;
			return FALSE;
		}
	}
	// ��������Ѿ�����ֱ��ɾ������
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
	// ���������Ƿ���ڶ�����ֹͣɾ������
	// ֹͣ���з���
	if (!Stop())
	{
		PRINTMSG(GetLastError(), "StopDriver LastError");
	}

	// ж�ط���
	if (!Uninstall())
	{
		PRINTMSG(GetLastError(), "UninstallDriver LastError");
	}

	PRINTMSG(POWER_SUCCESS, "UnloadDriver Success");
	return TRUE;
}



BOOL LoadDriver()
{
	// �Ȼ�ȡһ�������ļ�·��
	GetSysFullPath(_T("PowerReadWrite.sys"));

	// ��װ����
	if (Install())
	{
		// ��������
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
// ʹ�ÿ������������
BOOL IoCtrlDriver(DWORD dwIoCode, PVOID pInBuffer, DWORD dwInBufferLen, PVOID pOutBuffer, DWORD dwOutBufferLen, DWORD *RealRetBytes)
{
	DWORD dwRetByte;
	BOOL bRet = DeviceIoControl(g_hDevice, CTL_CODE_GEN(dwIoCode), pInBuffer, dwInBufferLen, pOutBuffer, dwOutBufferLen, &dwRetByte, NULL);
	if (RealRetBytes)
		*RealRetBytes = dwRetByte;
	return bRet;
}







// ��ȡ�����ļ�ȫ·��
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