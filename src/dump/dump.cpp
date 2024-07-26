/*
  felicalib - FeliCa access wrapper library

  Copyright (c) 2007, Takuya Murakami, All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are
  met:

  1. Redistributions of source code must retain the above copyright notice,
  this list of conditions and the following disclaimer. 

  2. Redistributions in binary form must reproduce the above copyright
  notice, this list of conditions and the following disclaimer in the
  documentation and/or other materials provided with the distribution. 

  3. Neither the name of the project nor the names of its contributors
  may be used to endorse or promote products derived from this software
  without specific prior written permission. 

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
/**
   @file dump.c

   FeliCa 僟儞僾
*/

#include <stdio.h>
#include <windows.h>
#include <shlwapi.h>
#include <mmsystem.h>
#include <tchar.h>
#include <locale>
#include <codecvt>
#include <locale.h>
#include <iostream>
#include "hookapi.h"

#include "felicalib.h"

#include "cxxopts.hpp"

#include "resource.h"

#ifdef _WIN64
#pragma comment(lib, "E:\\Works\\Felica\\Detours\\lib.X64\\detours.lib")
//#pragma comment(lib, "E:\\Works\\Felica\\Detours\\lib.X64\\syelog.lib")
#else
#pragma comment(lib, "E:\\Works\\Felica\\Detours\\lib.X32\\detours.lib")
//#pragma comment(lib, "E:\\Works\\Felica\\Detours\\lib.X32\\syelog.lib")
#endif
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "winmm.lib")

int glabel = 0;
static void printserviceinfo(uint16 s);
static void hexdump(uint8* addr, int n);
static void PrintID(TCHAR* key, uint8* addr, int n);

std::wstring GetLabelHub(int label){
	TCHAR apstHome[MAX_PATH];
	DWORD len = GetEnvironmentVariable(TEXT("APSTHOME"), apstHome, MAX_PATH);
	if (len == 0) {
		std::cerr << "Failed to get environment variable APSTHOME. Error: " << GetLastError() << std::endl;
		return _T("");
	}

	// 组合文件路径 FelicaCalibration.ini
	TCHAR iniFilePath[MAX_PATH];
	PathCombine(iniFilePath, apstHome, TEXT("FelicaCalibration.ini"));

	// 读取INI文件中Section="label", Key=1的Value
	TCHAR value[1024] = { 0 };
	GetPrivateProfileString(TEXT("label"), std::to_wstring(label).c_str(), TEXT(""), value, 1024, iniFilePath);

	// 检查是否成功读取
	if (lstrlen(value) == 0) {
		std::cerr << "Failed to read the value for key 'x' in section 'label'.\n";
		return _T("");
	}
	return value;
}

std::wstring devicename;
std::string devicenameA;
//std::list<std::string> FalicaSymblinks;
//int IndexSymblinks=0;
std::string WstringToString(const std::wstring& wstr)
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
	return converter.to_bytes(wstr);
}

int _tmain(int argc, _TCHAR *argv[])
{
	cxxopts::Options options(_T("Dump"), _T("Check whether Felica has data"));
	
	options.add_options()
		(_T("l,label"), _T("device label"), cxxopts::value<int>()->default_value(_T("0")))
		(_T("n,hubname"), _T("usb hub name"), cxxopts::value<std::wstring>())
		(_T("p,hubport"), _T("usb hub port"), cxxopts::value<int>()->default_value(_T("0")))
		(_T("d,dump"), _T("dump all data"), cxxopts::value<bool>()->default_value(_T("false")))
		(_T("s,sound"), _T("reader get card sound"), cxxopts::value<bool>()->default_value(_T("false")))
		(_T("h,help"), _T("Print usage:\n --hubname XXX --hubport XXX --label XXX"));

	
	auto result = options.parse(argc, argv);
	
	if (result.count(_T("help")))
	{
		std::wcout << options.help() << std::endl;
		exit(0);
	}
	
	std::wstring hubname;
	if (result.count(_T("hubname")))
		hubname = result[_T("hubname")].as<std::wstring>();
	int hubport = result[_T("hubport")].as<int>();
	int label = result[_T("label")].as<int>();
	bool bDump = result[_T("dump")].as<bool>();
	bool bSound = result[_T("sound")].as<bool>();
	glabel = label;
	bool busehubinfo = false;

ENUMDEVICE:
	//std::wcout << hubname << "@" << hubport << _T("           ") << label << std::endl;
	//L"USB#VID_2109&PID_2813#6&3183d08&0&1#{f18a0e88-c30c-11d0-8815-00a0c906bed8}"
	if (!hubname.empty() && hubport!=0){
		PrintDeivce(hubname, hubport, devicename);
		logIt(_T("get device path: %s"), devicename.c_str());
		devicenameA = WstringToString(devicename);
		
		busehubinfo = !devicename.empty();
	}
	if (!busehubinfo && label>0) {
		auto hubcal = GetLabelHub(label);
		// 定义分割符
		wchar_t delimiter = L'@';

		// 查找分割符的位置
		size_t delimiterPos = hubcal.find(delimiter);

		if (delimiterPos != std::wstring::npos) {
			// 提取分割符前的部分并转换为int
			std::wstring intPart = hubcal.substr(0, delimiterPos);
			hubport = std::stoi(intPart);

			// 提取分割符后的部分
			hubname = hubcal.substr(delimiterPos + 1);
			//logIt(_T("label path: %s==> %s"), intPart.c_str(), stringPart.c_str());

			PrintDeivce(hubname, hubport, devicename);
			logIt(_T("get device path: %s"), devicename.c_str());
			devicenameA = WstringToString(devicename);

			busehubinfo = !devicename.empty();
		}
		else {
			logIt(_T("Delimiter not found in the input string."));
		}
	}

	if (devicenameA.empty()&&(label>0|| hubport>0)) {

		auto felicaparentid = GetDeviceInstanceIDFromHubPort(hubname, hubport);
		if (felicaparentid.empty()) {
			logIt(_T("get device path is empty. reader not connect"));
			if (bSound) {
				PlaySound(MAKEINTRESOURCE(IDR_WAVE1), GetModuleHandle(NULL), SND_RESOURCE | SND_SYNC);
			}
			_tprintf(_T("Felicastatus=readernotfind"));
			exit(1);
		}
		RunExe(felicaparentid);
		Sleep(500);
		goto ENUMDEVICE;
	}

	//return 0;
    pasori* p;
    felica  *f, *f2;
    int i, j, k;
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

	setlocale(LC_ALL, "Japanese");

	if (busehubinfo) {
		SetupHook();
	}

    p = pasori_open(NULL);
    if (!p)
    {
        _ftprintf(stderr, _T("PaSoRi open failed.\n"));
		logIt(_T("PaSoRi open failed"));
		if (bSound) {
			PlaySound(MAKEINTRESOURCE(IDR_WAVE1), GetModuleHandle(NULL), SND_RESOURCE | SND_SYNC);
		}
		_tprintf(_T("Felicastatus=readernotfind"));

        exit(2);
    }
    pasori_init(p);

RETRY:

	BOOL bWait = true;

	do {
		f = felica_polling(p, POLLING_ANY, 0, 0);
		if (!f)
		{
			_ftprintf(stderr, _T("Polling card failed.\n"));
			logIt(_T("Polling card failed."));
			//exit(1);
			Sleep(250);
			continue;
		}
		bWait = false;
		/*logIt(_T("# IDm: "));
		hexdump(f->IDm, 8);*/
		logHex(f->IDm, 8, _T("# IDm: "));
		PrintID(_T("IDm"), f->IDm, 8);
		//logIt(_T("\n"));
		//logIt(_T("# PMm: "));
		//hexdump(f->PMm, 8);
		logHex(f->PMm, 8, _T("# PMm: "));
		PrintID(_T("PMm"), f->PMm, 8);
		//_tprintf(_T("\n\n"));
		felica_free(f);
	}while(bWait);


    f = felica_enum_systemcode(p);
	while (!f)
	{
		Sleep(500);
		f = felica_enum_systemcode(p);
	} 
	if (f->num_system_code == 0) {
		felica_free(f);
		goto RETRY;
	}
	/*if (!f)
    {
		SetConsoleTextAttribute(hConsole, FOREGROUND_RED);
		_tprintf(_T("Felicastatus=nodata\n"));
		logIt(_T("Felicastatus=nodata"));
		if (bSound) {
			PlaySound(MAKEINTRESOURCE(IDR_WAVE1), GetModuleHandle(NULL), SND_RESOURCE | SND_SYNC);
		}
		SetConsoleTextAttribute(hConsole, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);
        exit(0);
    }*/

	BOOL bServiceData = FALSE;

    for (i = 0; i < f->num_system_code; i++)
    {
		logIt(_T("# System code: %04X\n"), N2HS(f->system_code[i]));
        f2 = felica_enum_service(p, N2HS(f->system_code[i]));
        if (!f2)
        {
            _ftprintf(stderr, _T("Enum service failed.\n"));
			logIt(_T("Enum service failed."));
			_tprintf(_T("Felicastatus=data\n"));
			logIt(_T("Felicastatus=data"));
			if (bSound) {
				PlaySound(MAKEINTRESOURCE(IDR_WAVE1), GetModuleHandle(NULL), SND_RESOURCE | SND_SYNC);
			}
            exit(1);
        }
        
		logIt(_T("# Number of area = %d\n"), f2->num_area_code);
        for (j = 0; j < f2->num_area_code; j++)
        {
			logIt(_T("# Area: %04X - %04X\n"), f2->area_code[j], f2->end_service_code[j]);
        }            

		logIt(_T("# Number of service code = %d\n"), f2->num_service_code);
        for (j = 0; j < f2->num_service_code; j++)
        {
            uint16 service = f2->service_code[j];
            printserviceinfo(service);
			if ((service & 0x1) == 0) continue;

            for (k = 0; k < 255; k++)
            {
                uint8 data[16];

                if (felica_read_without_encryption02(f2, service, 0, (uint8)k, data))
                {
                    break;
                }
                
				logIt(_T("%04X:%04X "), service, k);
                hexdump(data, 16);
				logIt(_T("\n"));
				bServiceData = TRUE;
				if (!bDump) break;
            }
			if (!bDump && bServiceData) {
				break;
			}
        }
        _tprintf(_T("\n"));
        felica_free(f2);
    }

	if (f->num_system_code > 0 && bServiceData){
		SetConsoleTextAttribute(hConsole, FOREGROUND_BLUE);
		_tprintf(_T("Felicastatus=data\n"));
		logIt(_T("Felicastatus=data"));
		SetConsoleTextAttribute(hConsole, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);
		if (bSound) {
			PlaySound(MAKEINTRESOURCE(IDR_WAVE1), GetModuleHandle(NULL), SND_RESOURCE | SND_SYNC);
		}
	}
	else{
		SetConsoleTextAttribute(hConsole, FOREGROUND_RED);
		_tprintf(_T("Felicastatus=nodata\n"));
		logIt(_T("Felicastatus=nodata"));
		SetConsoleTextAttribute(hConsole, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);
		if (bSound) {
			PlaySound(MAKEINTRESOURCE(IDR_WAVE1), GetModuleHandle(NULL), SND_RESOURCE | SND_SYNC);
		}
	}

    felica_free(f);
    pasori_close(p);

	if (busehubinfo) {
		RemoveHook();
	}
    return 0;
}

static void printserviceinfo(uint16 s)
{
    TCHAR*  ident;

    switch ((s >> 1) & 0xf)
    {
        case 0: ident = _T("Area Code"); break;
        case 4: ident = _T("Random Access R/W"); break; 
        case 5: ident = _T("Random Access Read only"); break; 
        case 6: ident = _T("Cyclic Access R/W"); break; 
        case 7: ident = _T("Cyclic Access Read only"); break; 
        case 8: ident = _T("Purse (Direct)"); break;
        case 9: ident = _T("Purse (Cashback/decrement)"); break;
        case 10: ident = _T("Purse (Decrement)"); break;
        case 11: ident = _T("Purse (Read only)"); break;
        default: ident = _T("INVALID or UNKOWN"); break;
    }
	TCHAR buf[1024] = { 0 };
	_stprintf_s(buf, _T("# Serivce code = %04X : %s"), s, ident);
    if ((s & 0x1) == 0)
    {
		_stprintf_s(buf+_tcslen(buf),100, _T(" (Protected)"));
    }
	_stprintf_s(buf + _tcslen(buf),100, _T("\n"));
	logIt(buf);
}

static void hexdump(uint8* addr, int n)
{
    int i;
	TCHAR buf[1024] = { 0 };
    for (i = 0; i < n; i++)
    {
		_stprintf_s(buf+i*3, 1000-i*3, _T("%02X "), addr[i]);
    }
	logIt(buf);
}

static void PrintID(TCHAR* key, uint8* addr, int n)
{
	int i;
	TCHAR buf[1024] = { 0 };
	for (i = 0; i < n; i++)
	{
		_stprintf_s(buf + i * 2, 1000 - i * 2, _T("%02X"), addr[i]);
	}
	_tprintf(_T("%s=%s\r\n"), key, buf);
}
