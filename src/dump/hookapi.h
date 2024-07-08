#pragma once
#include <string>
void SetupHook();
void RemoveHook();
int PrintDeivce(std::wstring hubname, int hubport, std::wstring &devicepath);

void logIt(TCHAR* fmt, ...);

VOID
Oops
(
	__in PCTSTR File,
	ULONG Line,
	DWORD dwError
);

#define OOPS()		Oops(_T(__FILE__), __LINE__, GetLastError())
#define OOPSERR(d)	Oops(_T(__FILE__), __LINE__, d)
#define ENTER_FUNCTION()	logIt(_T("%s ++\n"), _T(__FUNCTION__))
#define EXIT_FUNCTION()		logIt(_T("%s --\n"), _T(__FUNCTION__))
#define EXIT_FUNCTRET(ret)		logIt(_T("%s -- return=%d\n"), _T(__FUNCTION__), ret)