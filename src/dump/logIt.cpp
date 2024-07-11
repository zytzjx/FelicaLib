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

void logHex(BYTE* buffer, size_t buf_sz, TCHAR *Title)
{
	logIt(Title);

	TCHAR line_buf[75];
	size_t pos = 0;
	int line_count = 0;
	while (pos < buf_sz)
	{
		int line_pos_1 = 0;
		int line_pos_2 = 0;
		ZeroMemory(line_buf, sizeof(line_buf));
		_stprintf_s(line_buf, _T("%04d:"), line_count++);
		line_buf[5] = ' ';
		line_pos_1 = 6;
		line_pos_2 = 54;
		for (size_t i = 0; i < 16 && pos < buf_sz; i++, pos++)
		{
			_stprintf_s(&line_buf[line_pos_1], 3, _T("%02x"), buffer[pos]);
			line_pos_1 += 2;
			line_buf[line_pos_1] = ' ';
			line_pos_1++;
			if (isprint(buffer[pos]))
				line_buf[line_pos_2] = (char)buffer[pos];
			else
				line_buf[line_pos_2] = '.';
			line_pos_2++;
		}
		for (; line_pos_1 < 54; line_pos_1++)
		{
			line_buf[line_pos_1] = ' ';
		}
		line_buf[line_pos_2] = 0;
		logIt(_T("%s\n"), line_buf);
	}
}