#pragma once

#include "../os.h"
#include "../utils/string-processing/string-processing.h"

namespace str_proc = string_processing;

#ifdef WIN_OS

#include <iostream>
#include "../logger/logger.h"

#include <windows.h>
#include <tchar.h>
#include <strsafe.h>
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "kernel32.lib")

#define SVCNAME L"Pritok Script Manager"
#define SVCDISPLAY L"Приток-А (Сервер Управляющих Скриптов)"
#define SVCDESCRIPTION L"Сервер управляющих скриптов. Предназначен для выполнения управляющих скриптов в виртуальном окружении"

#define WITHOUT_CONST(x) const_cast<char*>(x)
#define WITHOUT_CONST_W(x) const_cast<LPWSTR>(x)

extern SERVICE_STATUS          gSvcStatus;
extern SERVICE_STATUS_HANDLE   gSvcStatusHandle;
extern HANDLE                  ghSvcStopEvent;
extern const std::wstring	   gCommand;

VOID SvcInstall(int argc, WCHAR* argv[]);

VOID __stdcall DoStartSvc(void);
VOID __stdcall DoStopSvc(void);
VOID __stdcall DoDeleteSvc(void);
BOOL __stdcall StopDependentServices(SC_HANDLE&, SC_HANDLE&);

VOID WINAPI SvcCtrlHandler(DWORD);

VOID ReportSvcStatus(DWORD, DWORD, DWORD);
VOID SvcReportEvent(LPWSTR);

#endif