#include "stdafx.h"
#include <Windows.h>
#include <wchar.h>
#include <TlHelp32.h>
#include <stdlib.h>
#include <direct.h>
#include <stdio.h>
#include "resource.h"

#define GTAV_PROCESS_NAWE L"GTA5.exe"

INT_PTR CALLBACK DialogProc(_In_ HWND hwndDlg, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam);
VOID InjectDLL(CHAR* dllPath, TCHAR* wechatPath);
BOOL handleEventMsg(HWND hwndDlg, WPARAM wParam);


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    DialogBox(hInstance, MAKEINTRESOURCE(ID_MAIN), NULL, &DialogProc);
    return 0;
}

INT_PTR CALLBACK DialogProc(_In_ HWND hwndDlg, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:

		break;
	case WM_COMMAND:
		return handleEventMsg(hwndDlg, wParam);
		break;
	case WM_CLOSE:
		EndDialog(hwndDlg, 0);
		break;
	default:
		break;
	}
	return FALSE;

}


//界面事件处理函数
BOOL handleEventMsg(HWND hwndDlg, WPARAM wParam)
{
	CHAR paths[0x1000] = { 0 };
	TCHAR* buffer = NULL;
	TCHAR wechat_path[0x200] = { 0 };
	if ((buffer = _wgetcwd(NULL, 0)) == NULL)
	{
		perror("getcwd error");
	}
	else
	{
		sprintf_s(paths, "%ws\\GTAO_Booster.dll", buffer);
	}
	switch (wParam)
	{
	case INJECT_DLL:
		GetDlgItemText(hwndDlg, WECHAT_PATH, wechat_path, sizeof(wechat_path));
		InjectDLL(paths, wechat_path);
		return TRUE;
		break;
	default:
		break;
	}
	return FALSE;
}

DWORD ProcessNameFindPID(LPCWSTR ProcessName)
{
	HANDLE ProcessAll = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	PROCESSENTRY32 processInfo = { 0 };
	processInfo.dwSize = sizeof(PROCESSENTRY32);
	do
	{
		if (wcscmp(ProcessName, processInfo.szExeFile) == 0)
		{
			return processInfo.th32ProcessID;
		}
	} while (Process32Next(ProcessAll, &processInfo));
	return 0;
}

VOID InjectDLL(CHAR* dllPath, TCHAR* wechatPath)
{
	/*OutputDebugStringA(dllPath);
	OutputDebugString(wechatPath);*/
	DWORD PID = ProcessNameFindPID(GTAV_PROCESS_NAWE);
	if (PID == 0)
	{
		MessageBox(NULL, (LPCWSTR)L"GTA5没有运行或者找不到GTA5进程", (LPCWSTR)L"错误", 0);
		return;
	}
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, PID);
	if (NULL == hProcess)
	{
		MessageBox(NULL, (LPCWSTR)L"进程打开失败权限不足", (LPCWSTR)L"错误", 0);
		return;
	}

	LPVOID dllAdd = VirtualAllocEx(hProcess, NULL, sizeof(dllPath), MEM_COMMIT, PAGE_READWRITE);
	if (NULL == dllAdd)
	{
		MessageBox(NULL, (LPCWSTR)L"内存分配失败", (LPCWSTR)L"错误", 0);
		return;
	}

	if ((WriteProcessMemory(hProcess, dllAdd, dllPath, strlen(dllPath), NULL)) == 0)
	{
		MessageBox(NULL, (LPCWSTR)L"路径写入失败", (LPCWSTR)L"错误", 0);
		return;
	}
	LPVOID loadAdd = GetProcAddress(GetModuleHandle(L"Kernel32.dll"), "LoadLibraryA");
	HANDLE exec = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)loadAdd, dllAdd, 0, NULL);
	if (NULL == exec)
	{
		MessageBox(NULL, (LPCWSTR)L"注入失败", (LPCWSTR)L"错误", 0);
		return;
	}
	else {
		MessageBox(NULL, (LPCWSTR)L"注入成功", (LPCWSTR)L"提示", 0);
	}
}
