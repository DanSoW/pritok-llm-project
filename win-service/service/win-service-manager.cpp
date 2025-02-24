#include "win-service-manager.h"
#include <iostream>

#ifdef WIN_OS

SERVICE_STATUS          gSvcStatus;
SERVICE_STATUS_HANDLE   gSvcStatusHandle;
HANDLE                  ghSvcStopEvent = NULL;
const std::wstring      gCommand = L" /service";

/// <summary>
/// Installs a service in the SCM database
/// </summary>
/// <returns>None</returns>
VOID SvcInstall(int argc, WCHAR* argv[])
{
    SC_HANDLE schSCManager;
    SC_HANDLE schService;

    SERVICE_DESCRIPTIONW sd;
    LPWSTR szDesc = WITHOUT_CONST_W(SVCDESCRIPTION);

    std::vector<WCHAR> szUnquotedPath(SHRT_MAX);
    if (!GetModuleFileNameW(NULL, szUnquotedPath.data(), static_cast<DWORD>(szUnquotedPath.size())))
    {
        loggerError << (LogMsg() << "Установка службы завершилось ошибкой (" << GetLastError() << ")");
        return;
    }

    str_proc::trimVector<WCHAR, std::int16_t>(szUnquotedPath, WCHAR(0), 0);
    szUnquotedPath.insert(szUnquotedPath.end(), gCommand.begin(), gCommand.end());

    // Adding parameters
    for (std::int32_t i = 1; i < argc; i++) {
        std::wstring parameter = L" " + std::wstring(argv[i]);
        szUnquotedPath.insert(szUnquotedPath.end(), parameter.begin(), parameter.end());
    }

    logger << (LogMsgW() << L"Путь до исполняемого файла службы: " << szUnquotedPath.data());

    // In case the path contains a space, it must be quoted so that
    // it is correctly interpreted. For example,
    // "d:\my share\myservice.exe" should be specified as
    // ""d:\my share\myservice.exe"".

    /*std::vector<WCHAR> szPath(SHRT_MAX);
    StringCbPrintfW(szPath.data(), szPath.size(), L"\"%s\"", szUnquotedPath.data());
    str_proc::trimVector<WCHAR, std::int16_t>(szPath, WCHAR(0), 0);
    szUnquotedPath.clear();*/

    // Get a handle to the SCM database. 
    schSCManager = OpenSCManagerW(
        NULL,                    // local computer
        NULL,                    // ServicesActive database 
        SC_MANAGER_ALL_ACCESS);  // full access rights 

    if (schSCManager == NULL)
    {
        loggerError << (LogMsg() << "Получение доступа к менеджеру служб завершилось с ошибкой (" << GetLastError() << ")");
        return;
    }

    // Create the service
    schService = CreateServiceW(
        schSCManager,              // SCM database 
        SVCNAME,                   // name of service 
        SVCDISPLAY,                // service name to display 
        SERVICE_ALL_ACCESS,        // desired access 
        SERVICE_WIN32_OWN_PROCESS, // service type 
        SERVICE_AUTO_START,        // start type
        SERVICE_ERROR_NORMAL,      // error control type 
        szUnquotedPath.data(),             // path to service's binary 
        NULL,                      // no load ordering group 
        NULL,                      // no tag identifier 
        NULL,                      // no dependencies 
        NULL,                      // LocalSystem account 
        NULL);                     // no password 

    if (schService == NULL)
    {
        loggerError << (LogMsg() << "Создание службы завершилось с ошибкой (" << GetLastError() << ")");

        CloseServiceHandle(schSCManager);
        return;
    }
    else {
        sd.lpDescription = szDesc;
        if (!ChangeServiceConfig2W(
            schService,
            SERVICE_CONFIG_DESCRIPTION,
            &sd
        )) {
            loggerError << (LogMsg() << "Попытка изменить описание службы завершилось с ошибкой (" << GetLastError() << ")");
        }
        else {
            logger << (LogMsg() << "Описание службы успешно изменено.");
        }

        logger << (LogMsgW() << L"Служба \"" << SVCNAME << L"\" успешно установлена.");
    }

    CloseServiceHandle(schService);
    CloseServiceHandle(schSCManager);
}

/// <summary>
/// Deletes a service from the SCM database
/// </summary>
/// <returns>None</returns>
VOID __stdcall DoDeleteSvc()
{
    SC_HANDLE schSCManager;
    SC_HANDLE schService;
    // SERVICE_STATUS ssStatus;

    // Get a handle to the SCM database. 
    schSCManager = OpenSCManagerW(
        NULL,                    // local computer
        NULL,                    // ServicesActive database 
        SC_MANAGER_ALL_ACCESS);  // full access rights 

    if (NULL == schSCManager)
    {
        loggerError << (LogMsg() << "Получение доступа к менеджеру служб завершилось с ошибкой (" << GetLastError() << ")");
        return;
    }

    // Get a handle to the service.
    schService = OpenServiceW(
        schSCManager,       // SCM database 
        SVCNAME,            // name of service 
        DELETE);            // need delete access 

    if (schService == NULL)
    {
        loggerError << (LogMsg() << "Получение доступа к службе завершилось с ошибкой (" << GetLastError() << ")");

        CloseServiceHandle(schSCManager);
        return;
    }

    // Delete the service.
    if (!DeleteService(schService))
    {
        loggerError << (LogMsg() << "Удаление службы завершилось с ошибкой (" << GetLastError() << ")");
    }
    else {
        logger << "Служба успешно удалена";
    }

    CloseServiceHandle(schService);
    CloseServiceHandle(schSCManager);
}


/// <summary>
/// Starts the service if possible
/// </summary>
/// <returns>None</returns>
VOID __stdcall DoStartSvc()
{
    SERVICE_STATUS_PROCESS ssStatus;
    ULONGLONG dwOldCheckPoint;
    ULONGLONG dwStartTickCount;
    DWORD dwWaitTime;
    DWORD dwBytesNeeded;

    SC_HANDLE schSCManager;
    SC_HANDLE schService;

    // Get a handle to the SCM database. 
    schSCManager = OpenSCManagerW(
        NULL,                    // local computer
        NULL,                    // servicesActive database 
        SC_MANAGER_ALL_ACCESS);  // full access rights 

    if (NULL == schSCManager)
    {
        loggerError << (LogMsg() << "Получение доступа к менеджеру служб завершилось с ошибкой (" << GetLastError() << ")");
        return;
    }

    // Get a handle to the service.
    schService = OpenServiceW(
        schSCManager,         // SCM database 
        SVCNAME,              // name of service 
        SERVICE_ALL_ACCESS);  // full access 

    if (schService == NULL)
    {
        loggerError << (LogMsg() << "Получение доступа к службе завершилось с ошибкой (" << GetLastError() << ")");
        CloseServiceHandle(schSCManager);
        return;
    }

    // Check the status in case the service is not stopped.
    if (!QueryServiceStatusEx(
        schService,                     // handle to service 
        SC_STATUS_PROCESS_INFO,         // information level
        (LPBYTE)&ssStatus,              // address of structure
        sizeof(SERVICE_STATUS_PROCESS), // size of structure
        &dwBytesNeeded))                // size needed if buffer is too small
    {
        loggerError << (LogMsg() << "Получение текущего статуса службы завершилось с ошибкой (" << GetLastError() << ")");

        CloseServiceHandle(schService);
        CloseServiceHandle(schSCManager);
        return;
    }

    // Check if the service is already running. It would be possible 
    // to stop the service here, but for simplicity this example just returns. 
    if (ssStatus.dwCurrentState != SERVICE_STOPPED && ssStatus.dwCurrentState != SERVICE_STOP_PENDING)
    {
        loggerError << (LogMsgW() << L"Нельзя запустить службу \"" << SVCNAME << L"\", поскольку она уже запущена.");

        CloseServiceHandle(schService);
        CloseServiceHandle(schSCManager);
        return;
    }

    // Save the tick count and initial checkpoint.
    dwStartTickCount = GetTickCount64();
    dwOldCheckPoint = ssStatus.dwCheckPoint;

    // Wait for the service to stop before attempting to start it.
    while (ssStatus.dwCurrentState == SERVICE_STOP_PENDING)
    {
        // Do not wait longer than the wait hint. A good interval is 
        // one-tenth of the wait hint but not less than 1 second  
        // and not more than 10 seconds. 

        dwWaitTime = ssStatus.dwWaitHint / 10;

        if (dwWaitTime < 1000) {
            dwWaitTime = 1000;
        }
        else if (dwWaitTime > 10000) {
            dwWaitTime = 10000;
        }

        Sleep(dwWaitTime);

        // Check the status until the service is no longer stop pending. 
        if (!QueryServiceStatusEx(
            schService,                     // handle to service 
            SC_STATUS_PROCESS_INFO,         // information level
            (LPBYTE)&ssStatus,             // address of structure
            sizeof(SERVICE_STATUS_PROCESS), // size of structure
            &dwBytesNeeded))              // size needed if buffer is too small
        {
            loggerError << (LogMsg() << "Получение текущего статуса службы завершилось с ошибкой (" << GetLastError() << ")");

            CloseServiceHandle(schService);
            CloseServiceHandle(schSCManager);
            return;
        }

        if (ssStatus.dwCheckPoint > dwOldCheckPoint)
        {
            // Continue to wait and check.
            dwStartTickCount = GetTickCount64();
            dwOldCheckPoint = ssStatus.dwCheckPoint;
        }
        else
        {
            if ((GetTickCount64() - dwStartTickCount) > (ULONGLONG)ssStatus.dwWaitHint)
            {
                loggerError << "Тайм-аут ожидания остановки службы подошёл к концу.";

                CloseServiceHandle(schService);
                CloseServiceHandle(schSCManager);
                return;
            }
        }
    }

    // Attempt to start the service.
    if (!StartServiceW(
        schService,  // handle to service 
        0,           // number of arguments 
        NULL))       // no arguments 
    {
        loggerError << (LogMsg() << "Запуск службы завершился с ошибкой (" << GetLastError() << ")");

        CloseServiceHandle(schService);
        CloseServiceHandle(schSCManager);
        return;
    }
    else {
        logger << "Ожидание запуска службы ...";
    }

    // Check the status until the service is no longer start pending. 
    if (!QueryServiceStatusEx(
        schService,                     // handle to service 
        SC_STATUS_PROCESS_INFO,         // info level
        (LPBYTE)&ssStatus,             // address of structure
        sizeof(SERVICE_STATUS_PROCESS), // size of structure
        &dwBytesNeeded))              // if buffer too small
    {
        loggerError << (LogMsg() << "Получение текущего статуса службы завершилось с ошибкой (" << GetLastError() << ")");

        CloseServiceHandle(schService);
        CloseServiceHandle(schSCManager);
        return;
    }

    // Save the tick count and initial checkpoint.
    dwStartTickCount = GetTickCount64();
    dwOldCheckPoint = ssStatus.dwCheckPoint;

    while (ssStatus.dwCurrentState == SERVICE_START_PENDING)
    {
        // Do not wait longer than the wait hint. A good interval is 
        // one-tenth the wait hint, but no less than 1 second and no 
        // more than 10 seconds. 

        dwWaitTime = ssStatus.dwWaitHint / 10;

        if (dwWaitTime < 1000) {
            dwWaitTime = 1000;
        }
        else if (dwWaitTime > 10000) {
            dwWaitTime = 10000;
        }

        Sleep(dwWaitTime);

        // Check the status again. 
        if (!QueryServiceStatusEx(
            schService,             // handle to service 
            SC_STATUS_PROCESS_INFO, // info level
            (LPBYTE)&ssStatus,             // address of structure
            sizeof(SERVICE_STATUS_PROCESS), // size of structure
            &dwBytesNeeded))              // if buffer too small
        {
            loggerError << (LogMsg() << "Получение текущего статуса службы завершилось с ошибкой (" << GetLastError() << ")");
            break;
        }

        if (ssStatus.dwCheckPoint > dwOldCheckPoint)
        {
            // Continue to wait and check.
            dwStartTickCount = GetTickCount64();
            dwOldCheckPoint = ssStatus.dwCheckPoint;
        }
        else
        {
            if ((GetTickCount64() - dwStartTickCount) > (ULONGLONG)ssStatus.dwWaitHint)
            {
                // No progress made within the wait hint.
                break;
            }
        }
    }

    // Determine whether the service is running.

    if (ssStatus.dwCurrentState == SERVICE_RUNNING)
    {
        logger << "Служба успешно запущена";
    }
    else
    {
        loggerError << "Служба не запущена";
        loggerError << (LogMsg() << "Current State: " << ssStatus.dwCurrentState);
        loggerError << (LogMsg() << "Exit Code: " << ssStatus.dwWin32ExitCode);
        loggerError << (LogMsg() << "Check Point: " << ssStatus.dwCheckPoint);
        loggerError << (LogMsg() << "Wait Hint: " << ssStatus.dwWaitHint);
    }

    CloseServiceHandle(schService);
    CloseServiceHandle(schSCManager);
}

/// <summary>
/// Stops the service
/// </summary>
/// <returns>None</returns>
VOID __stdcall DoStopSvc()
{
    SERVICE_STATUS_PROCESS ssp;
    ULONGLONG dwStartTime = GetTickCount64();
    DWORD dwBytesNeeded;
    ULONGLONG dwTimeout = 30000; // 30-second time-out
    DWORD dwWaitTime;

    SC_HANDLE schSCManager;
    SC_HANDLE schService;

    // Get a handle to the SCM database. 
    schSCManager = OpenSCManagerW(
        NULL,                    // local computer
        NULL,                    // ServicesActive database 
        SC_MANAGER_ALL_ACCESS);  // full access rights 

    if (NULL == schSCManager)
    {
        loggerError << (LogMsg() << "Получение доступа к менеджеру служб завершилось с ошибкой (" << GetLastError() << ")");
        return;
    }

    // Get a handle to the service.
    schService = OpenServiceW(
        schSCManager,         // SCM database 
        SVCNAME,            // name of service 
        SERVICE_STOP |
        SERVICE_QUERY_STATUS |
        SERVICE_ENUMERATE_DEPENDENTS);

    if (schService == NULL)
    {
        loggerError << (LogMsg() << "Получение доступа к службе завершилось с ошибкой (" << GetLastError() << ")");
        CloseServiceHandle(schSCManager);
        return;
    }

    // Make sure the service is not already stopped.
    if (!QueryServiceStatusEx(
        schService,
        SC_STATUS_PROCESS_INFO,
        (LPBYTE)&ssp,
        sizeof(SERVICE_STATUS_PROCESS),
        &dwBytesNeeded))
    {
        loggerError << (LogMsg() << "Получение текущего статуса службы завершилось с ошибкой (" << GetLastError() << ")");

        CloseServiceHandle(schService);
        CloseServiceHandle(schSCManager);

        return;
    }

    if (ssp.dwCurrentState == SERVICE_STOPPED)
    {
        loggerError << (LogMsg() << "Служба уже остановлена");

        CloseServiceHandle(schService);
        CloseServiceHandle(schSCManager);

        return;
    }

    if (ssp.dwCurrentState == SERVICE_STOP_PENDING) {
        logger << "Служба уже в состоянии остановки";
    }

    // If a stop is pending, wait for it.
    while (ssp.dwCurrentState == SERVICE_STOP_PENDING)
    {
        logger << "Ожидание остановки службы ...";

        // Do not wait longer than the wait hint. A good interval is 
        // one-tenth of the wait hint but not less than 1 second  
        // and not more than 10 seconds. 
        dwWaitTime = ssp.dwWaitHint / 10;

        if (dwWaitTime < 1000) {
            dwWaitTime = 1000;
        }
        else if (dwWaitTime > 10000) {
            dwWaitTime = 10000;
        }

        Sleep(dwWaitTime);

        if (!QueryServiceStatusEx(
            schService,
            SC_STATUS_PROCESS_INFO,
            (LPBYTE)&ssp,
            sizeof(SERVICE_STATUS_PROCESS),
            &dwBytesNeeded))
        {
            loggerError << (LogMsg() << "Получение текущего статуса службы завершилось с ошибкой (" << GetLastError() << ")");

            CloseServiceHandle(schService);
            CloseServiceHandle(schSCManager);

            return;
        }

        if (ssp.dwCurrentState == SERVICE_STOPPED)
        {
            logger << "Служба успешно остановлена";

            CloseServiceHandle(schService);
            CloseServiceHandle(schSCManager);

            return;
        }

        if ((GetTickCount64() - dwStartTime) > dwTimeout)
        {
            loggerError << (LogMsg() << "Время ожидания остановки службы подошло к концу. Служба не остановлена.");

            CloseServiceHandle(schService);
            CloseServiceHandle(schSCManager);
            return;
        }
    }

    // If the service is running, dependencies must be stopped first.
    StopDependentServices(schSCManager, schService);

    // Send a stop code to the service.
    if (!ControlService(
        schService,
        SERVICE_CONTROL_STOP,
        (LPSERVICE_STATUS)&ssp))
    {
        loggerError << (LogMsg() << "Отправка службе команды об остановке завершилась с ошибкой (" << GetLastError() << ")");

        CloseServiceHandle(schService);
        CloseServiceHandle(schSCManager);

        return;
    }

    // Wait for the service to stop.
    while (ssp.dwCurrentState != SERVICE_STOPPED)
    {
        Sleep(ssp.dwWaitHint);

        if (!QueryServiceStatusEx(
            schService,
            SC_STATUS_PROCESS_INFO,
            (LPBYTE)&ssp,
            sizeof(SERVICE_STATUS_PROCESS),
            &dwBytesNeeded))
        {
            loggerError << (LogMsg() << "Получение текущего статуса службы завершилось с ошибкой (" << GetLastError() << ")");

            CloseServiceHandle(schService);
            CloseServiceHandle(schSCManager);

            return;
        }

        if (ssp.dwCurrentState == SERVICE_STOPPED)
            break;

        if ((GetTickCount64() - dwStartTime) > dwTimeout)
        {
            loggerError << (LogMsg() << "Время ожидания остановки службы подошло к концу. Служба не остановлена.");

            CloseServiceHandle(schService);
            CloseServiceHandle(schSCManager);
            return;
        }
    }

    logger << "Служба успешно остановлена";

    CloseServiceHandle(schService);
    CloseServiceHandle(schSCManager);
}

/// <summary>
/// Stopping all dependent services
/// </summary>
/// <param name="schSCManager">Service Manager</param>
/// <param name="schService">Service</param>
/// <returns></returns>
BOOL __stdcall StopDependentServices(SC_HANDLE& schSCManager, SC_HANDLE& schService)
{
    DWORD i;
    DWORD dwBytesNeeded;
    DWORD dwCount;

    LPENUM_SERVICE_STATUSW  lpDependencies = NULL;
    ENUM_SERVICE_STATUSW    ess;
    SC_HANDLE               hDepService;
    SERVICE_STATUS_PROCESS  ssp;

    ULONGLONG dwStartTime = GetTickCount64();
    ULONGLONG dwTimeout = 30000; // 30-second time-out

    // Pass a zero-length buffer to get the required buffer size.
    if (EnumDependentServicesW(schService, SERVICE_ACTIVE,
        lpDependencies, 0, &dwBytesNeeded, &dwCount))
    {
        // If the Enum call succeeds, then there are no dependent
        // services, so do nothing.
        return TRUE;
    }
    else
    {
        DWORD error = GetLastError();
        if (error != ERROR_MORE_DATA) {
            loggerError << (LogMsg() << "Возникла непредвиденная ошибка (" << error << ")");

            return FALSE; // Unexpected error
        }

        // Allocate a buffer for the dependencies.
        lpDependencies = (LPENUM_SERVICE_STATUSW)HeapAlloc(
            GetProcessHeap(), HEAP_ZERO_MEMORY, dwBytesNeeded);

        if (!lpDependencies) {
            return FALSE;
        }

        // Enumerate the dependencies.
        if (!EnumDependentServicesW(schService, SERVICE_ACTIVE,
            lpDependencies, dwBytesNeeded, &dwBytesNeeded,
            &dwCount)) {
            return FALSE;
        }

        for (i = 0; i < dwCount; i++)
        {
            ess = *(lpDependencies + i);

            // Open the service.
            hDepService = OpenServiceW(schSCManager,
                ess.lpServiceName,
                SERVICE_STOP | SERVICE_QUERY_STATUS);

            if (!hDepService) {
                return FALSE;
            }

            // Send a stop code.
            if (!ControlService(hDepService,
                SERVICE_CONTROL_STOP,
                (LPSERVICE_STATUS)&ssp)) {
                return FALSE;
            }

            // Wait for the service to stop.
            while (ssp.dwCurrentState != SERVICE_STOPPED)
            {
                Sleep(ssp.dwWaitHint);
                if (!QueryServiceStatusEx(
                    hDepService,
                    SC_STATUS_PROCESS_INFO,
                    (LPBYTE)&ssp,
                    sizeof(SERVICE_STATUS_PROCESS),
                    &dwBytesNeeded)) {
                    return FALSE;
                }

                if (ssp.dwCurrentState == SERVICE_STOPPED) {
                    break;
                }

                if ((GetTickCount64() - dwStartTime) > dwTimeout) {
                    return FALSE;
                }
            }

            // Always release the service handle.
            CloseServiceHandle(hDepService);
        }

        // Always free the enumeration buffer.
        HeapFree(GetProcessHeap(), 0, lpDependencies);
    }

    return TRUE;
}

/// <summary>
/// Logs messages to the event log
/// </summary>
/// <param name="szFunction">name of function that failed</param>
/// <returns>None</returns>
VOID SvcReportEvent(LPWSTR szFunction)
{
    HANDLE hEventSource;
    LPCWSTR lpszStrings[2];
    WCHAR Buffer[80];

    hEventSource = RegisterEventSourceW(NULL, SVCNAME);

    if (NULL != hEventSource)
    {
        StringCchPrintfW(Buffer, 80, L"%s failed with %d", szFunction, GetLastError());

        lpszStrings[0] = SVCNAME;
        lpszStrings[1] = Buffer;

        ReportEventW(
            hEventSource,        // event log handle
            EVENTLOG_ERROR_TYPE, // event type
            0,                   // event category
            1,//SVC_ERROR,       // event identifier
            NULL,                // no security identifier
            2,                   // size of lpszStrings array
            0,                   // no binary data
            lpszStrings,         // array of strings
            NULL                 // no binary data
        );

        DeregisterEventSource(hEventSource);
    }
}

/// <summary>
/// Sets the current service status and reports it to the SCM
/// </summary>
/// <param name="dwCurrentState">The current state (see SERVICE_STATUS)</param>
/// <param name="dwWin32ExitCode">The system error code</param>
/// <param name="dwWaitHint">Estimated time for pending operation, in milliseconds</param>
/// <returns>None</returns>
VOID ReportSvcStatus(
    DWORD dwCurrentState,
    DWORD dwWin32ExitCode,
    DWORD dwWaitHint
)
{
    static DWORD dwCheckPoint = 1;

    // Fill in the SERVICE_STATUS structure.
    gSvcStatus.dwCurrentState = dwCurrentState;
    gSvcStatus.dwWin32ExitCode = dwWin32ExitCode;
    gSvcStatus.dwWaitHint = dwWaitHint;

    if (dwCurrentState == SERVICE_START_PENDING) {
        gSvcStatus.dwControlsAccepted = 0;
    }
    else {
        gSvcStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
    }

    if ((dwCurrentState == SERVICE_RUNNING) ||
        (dwCurrentState == SERVICE_STOPPED)) {
        gSvcStatus.dwCheckPoint = 0;
    }
    else {
        gSvcStatus.dwCheckPoint = dwCheckPoint++;
    }

    // Report the status of the service to the SCM.
    SetServiceStatus(gSvcStatusHandle, &gSvcStatus);
}


/// <summary>
/// Called by SCM whenever a control code is sent to the service
/// using the ControlService function
/// </summary>
/// <param name="dwCtrl">Control code</param>
/// <returns>None</returns>
VOID WINAPI SvcCtrlHandler(DWORD dwCtrl)
{
    logger << (LogMsg() << "(SvcCtrlHandler) Thread ID: " << std::this_thread::get_id());

    // Handle the requested control code. 
    switch (dwCtrl)
    {
    case SERVICE_CONTROL_STOP:
        ReportSvcStatus(SERVICE_STOP_PENDING, NO_ERROR, 0);

        // Signal the service to stop.
        SetEvent(ghSvcStopEvent);
        ReportSvcStatus(gSvcStatus.dwCurrentState, NO_ERROR, 0);


        logger << "SERVICE_CONTROL_STOP";
        return;

    case SERVICE_CONTROL_INTERROGATE:
        logger << "SERVICE_CONTROL_INTERROGATE";
        break;

    default:
        break;
    }

}

#endif