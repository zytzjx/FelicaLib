#include <windows.h>
#include <stdio.h>
#include <string>
#include <tchar.h>

extern int glabel;


VOID
Oops(
	__in PCTSTR File,
	ULONG Line,
	DWORD dwError)
{
	TCHAR szBuf[2048] = { 0 };

	_stprintf_s(szBuf, _T("[LABEL_%d]File: %ws, Line %d, Error %d\n"), glabel, File, Line, dwError);
	OutputDebugString(szBuf);
}

void logIt(TCHAR* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	int size = _vscwprintf(fmt, args) + 1; // Get the size of the formatted string
	std::wstring buffer(size, L'\0'); // Create a buffer with necessary size
	vswprintf_s(&buffer[0], size, fmt, args); // Format the string
	va_end(args);

	std::wstring formattedString = L"[Label_" + std::to_wstring(glabel) + L"]: " + buffer;
	OutputDebugString(formattedString.c_str());
}