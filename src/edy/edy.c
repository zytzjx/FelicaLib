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
  @file edy.c

  edy �����_���v
*/

#include <stdio.h>
#include <time.h>
#include <windows.h>
#include <tchar.h>
#include <locale.h>

#include "felicalib.h"

static void edy_dump(uint8 *data);
static void analyzeTime(int n, struct tm *t);
static int read4b(uint8 *p);
static int read2b(uint8 *p);

// �T�[�r�X�R�[�h
#define SERVICE_EDY     0x170f


int _tmain(int argc, _TCHAR *argv[])
{
    pasori *p;
    felica *f;
    int i;
    uint8 data[16];

    setlocale( LC_ALL, "Japanese");

    p = pasori_open(NULL);
    if (!p)
    {
        _ftprintf(stderr, _T("PaSoRi open failed.\n"));
    exit(1);
    }
    pasori_init(p);
    
    f = felica_polling(p, POLLING_EDY, 0, 0);
    if (!f)
    {
        _ftprintf(stderr, _T("Polling card failed.\n"));
        exit(1);
    }

    _tprintf(_T("IDm: "));
    for (i = 0; i < 8; i++)
    {
        _tprintf(_T("%02x"), f->IDm[i]);
    }
    _tprintf(_T("\n"));

    for (i = 0; ; i++)
    {
        if (felica_read_without_encryption02(f, SERVICE_EDY, 0, (uint8)i, data))
        {
            break;
        }
        edy_dump(data);
    }
    felica_free(f);
    pasori_close(p);

    return 0;
}

static void edy_dump(uint8 *data)
{
    int proc, time, value, balance, seq, v;
    struct  tm tt;

    v = read4b(data + 0);
    proc = v >> 24;         // ����
    seq  = v & 0xffffff;        // �A��
    time = read4b(data + 4);    // ����
    value = read4b(data + 8);   // ���z
    balance = read4b(data + 12);    // �c��        

    // ���t/����
    analyzeTime(time, &tt);
    _tprintf(_T("%d/%02d/%02d %02d:%02d:%02d "),
        tt.tm_year, tt.tm_mon, tt.tm_mday,
        tt.tm_hour, tt.tm_min, tt.tm_sec);

    switch (proc)
    {
        case 0x02:
            _tprintf(_T("�`���[�W "));
            break;
        case 0x20:
            _tprintf(_T("�x����   "));
            break;
        case 0x04:
            _tprintf(_T("�M�t�g   "));
            break;
        default:
            _tprintf(_T("????     "));
            break;
    }

    _tprintf(_T("���z:%-5d "), value);
    _tprintf(_T("�c��:%-5d "), balance);
    _tprintf(_T("�A��:%d\n"), seq);
}

static void analyzeTime(int n, struct tm *t)
{
    time_t  tt;
	struct  tm t2 = { 0 };

    // calculate day
    memset(t, 0, sizeof(*t));
    t->tm_year  = 2000 - 1900;
    t->tm_mon   = 0;
    t->tm_mday  = 1;
    t->tm_hour  = 0;
    t->tm_min   = 0;
    t->tm_sec   = 0;
    t->tm_isdst = -1;

    tt = mktime(t);
    tt += (n >> 17) * 24 * 60 * 60;

    localtime_s(&t2, &tt);
    memcpy(t, &t2, sizeof(*t));

    t->tm_year += 1900;
    t->tm_mon += 1;

    // calculate time
    n = n & 0x1ffff;
    t->tm_sec = n % 60;
    n /= 60;
    t->tm_min = n % 60;
    t->tm_hour = n / 60;
}

static int read4b(uint8 *p)
{
    int v;
    v = (*p++) << 24;
    v |= (*p++) << 16;
    v |= (*p++) << 8;
    v |= *p;
    return v;
}

static int read2b(uint8 *p)
{
    int v;
    v = (*p++) << 8;
    v |= *p;
    return v;
}


