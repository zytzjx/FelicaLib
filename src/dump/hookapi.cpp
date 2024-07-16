//
#include <windows.h>
#include <Shlwapi.h>
#include <detours.h>
#include <fstream>
#include <iostream>
#include "hookapi.h"

#pragma comment(lib, "Shlwapi.lib")

//////////////////////////////////////////////////////////////////////////////
//
extern "C" {
	HANDLE(WINAPI * Real_CreateFileW)(LPCWSTR a0,
		DWORD a1,
		DWORD a2,
		LPSECURITY_ATTRIBUTES a3,
		DWORD a4,
		DWORD a5,
		HANDLE a6)
		= CreateFileW;

	HANDLE(WINAPI * Real_CreateFileA)(LPCSTR a0,
		DWORD a1,
		DWORD a2,
		LPSECURITY_ATTRIBUTES a3,
		DWORD a4,
		DWORD a5,
		HANDLE a6)
		= CreateFileA;

	BOOL(WINAPI * Real_DeviceIoControl)(
		HANDLE       hDevice,
		DWORD        dwIoControlCode,
		LPVOID       lpInBuffer,
		DWORD        nInBufferSize,
		LPVOID       lpOutBuffer,
		DWORD        nOutBufferSize,
		LPDWORD      lpBytesReturned,
		LPOVERLAPPED lpOverlapped) 
	= DeviceIoControl;

}

BOOL __stdcall Mine_DeviceIoControl(
	HANDLE       hDevice,
	DWORD        dwIoControlCode,
	LPVOID       lpInBuffer,
	DWORD        nInBufferSize,
	LPVOID       lpOutBuffer,
	DWORD        nOutBufferSize,
	LPDWORD      lpBytesReturned,
	LPOVERLAPPED lpOverlapped) {
	logHex((BYTE *)lpInBuffer, nInBufferSize, (L"Enter DeviceIoControl:"));
	BOOL b = Real_DeviceIoControl(hDevice,
		       dwIoControlCode,
		      lpInBuffer,
		        nInBufferSize,
		       lpOutBuffer,
		        nOutBufferSize,
		      lpBytesReturned,
		 lpOverlapped);
	logHex((BYTE *)lpOutBuffer, nOutBufferSize, (L"Exit DeviceIoControl:"));
	return b;
}

HANDLE __stdcall Mine_CreateFileA(LPCSTR a0,
	DWORD a1,
	DWORD a2,
	LPSECURITY_ATTRIBUTES a3,
	DWORD a4,
	DWORD a5,
	HANDLE a6)
{
	OutputDebugStringA(a0);
	//\\?\usb#vid_054c&pid_0dc9&mi_00#
	extern std::string devicenameA;
	CHAR filename[1024] = { 0 };
	BOOL bDevice = false;
	if (StrStrIA(a0, "\\\\?\\usb#vid_054c&pid_0dc") != nullptr) {
		//extern int IndexSymblinks;//std::list<std::string> FalicaSymblinks;
		//sprintf_s(filename, "%s", devicenameA.c_str());
		//a0 = filename;
		bDevice = true;
	}

	HANDLE rv = 0;
	try {
		if (bDevice) {
			for (int i = 0; i < 99; i++) {
				sprintf_s(filename, "%s\\U*%02d", devicenameA.c_str(),i);
				rv = Real_CreateFileA(filename, a1, a2, a3, a4, a5, a6);
				if (rv != INVALID_HANDLE_VALUE) {
					break;
				}
			}
		}
		else {
			rv = Real_CreateFileA(a0, a1, a2, a3, a4, a5, a6);
		}
	}
	catch(...) {
	};
	if (bDevice) {
		sprintf_s(filename, "%s==>%d", filename, GetLastError());
		OutputDebugStringA(filename);
	}
	return rv;
}

HANDLE __stdcall Mine_CreateFileW(LPCWSTR a0,
	DWORD a1,
	DWORD a2,
	LPSECURITY_ATTRIBUTES a3,
	DWORD a4,
	DWORD a5,
	HANDLE a6)
{
	//extern std::wstring devicename;
	TCHAR filename[1024] = { 0 };
	/*if (StrStrIW(a0, L"\\\\?\\usb#vid_054c&pid_0dc9&mi_00#") != nullptr) {
		swprintf_s(filename, L"%s", devicename.c_str());
		a0 = filename;
	}*/
	OutputDebugStringW(a0);
	//\\?\usb#vid_054c&pid_0dc9&mi_00#
	HANDLE rv = 0;
	try {
		rv = Real_CreateFileW(a0, a1, a2, a3, a4, a5, a6);
	}
	catch(...) {
	};
	swprintf_s(filename, L"W:%s==>%d", a0, GetLastError());
	OutputDebugStringW(filename);
	return rv;
}

// Function to set up the hook
void SetupHook() {
	DetourRestoreAfterWith();
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	//DetourAttach(&(PVOID&)Real_CreateFileW, Mine_CreateFileW);
	DetourAttach(&(PVOID&)Real_CreateFileA, Mine_CreateFileA);
	//DetourAttach(&(PVOID&)Real_DeviceIoControl, Mine_DeviceIoControl);
	DetourTransactionCommit();
}

// Function to remove the hook
void RemoveHook() {
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	//DetourDetach(&(PVOID&)Real_CreateFileW, Mine_CreateFileW);
	DetourDetach(&(PVOID&)Real_CreateFileA, Mine_CreateFileA);
	//DetourAttach(&(PVOID&)Real_DeviceIoControl, Mine_DeviceIoControl);
	DetourTransactionCommit();
}
