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
}

HANDLE __stdcall Mine_CreateFileA(LPCSTR a0,
	DWORD a1,
	DWORD a2,
	LPSECURITY_ATTRIBUTES a3,
	DWORD a4,
	DWORD a5,
	HANDLE a6)
{
	//\\?\usb#vid_054c&pid_0dc9&mi_00#
	extern std::string devicenameA;
	CHAR filename[1024] = { 0 };
	if (StrStrIA(a0, "\\\\?\\usb#vid_054c&pid_0dc9&mi_00#") != nullptr) {
		sprintf_s(filename, "%s\\U*00", devicenameA.c_str());
		a0 = filename;
	}
	OutputDebugStringA(a0);

	HANDLE rv = 0;
	try {
		rv = Real_CreateFileA(a0, a1, a2, a3, a4, a5, a6);
	}
	catch(...) {
	};
	sprintf_s(filename, "A:%s==>%d", a0, GetLastError());
	OutputDebugStringA(filename);

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
	DetourTransactionCommit();
}

// Function to remove the hook
void RemoveHook() {
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	//DetourDetach(&(PVOID&)Real_CreateFileW, Mine_CreateFileW);
	DetourDetach(&(PVOID&)Real_CreateFileA, Mine_CreateFileA);
	DetourTransactionCommit();
}
