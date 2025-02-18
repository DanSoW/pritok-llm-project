#define _DISABLE_CONSTEXPR_MUTEX_CONSTRUCTOR
#define NOMINMAX
#include "win-service.h"
#include "conf-env/conf-env.h"
#include "logger/logger.h"
#include <string.h>
#include <atomic>
#include <thread>
#include <chrono>
#include "utils/process-info/process-info.h"
#include "utils/memory-control/memory-control.h"
#include "utils/console/console.h"
#include "packages/datetime/datetime.h"
#include "packages/file-watcher/file-watcher.h"
#include <ctime>
#include <cstddef>

using namespace std;

#ifdef WIN_OS
#include "service/win-service-manager.h"

VOID SvcInit(DWORD, LPWSTR*);
VOID WINAPI SvcMain(DWORD, LPWSTR*);

void mainFunc(int argc, TCHAR* argv[]) {
    if (lstrcmpi(argv[1], TEXT("/install")) == 0) {
        SvcInstall();

        std::cout << "Служба установлена" << std::endl;
    }
    else if (lstrcmpi(argv[1], TEXT("/start")) == 0) {
        DoStartSvc();

        std::cout << "Служба запущена" << std::endl;
    }
    else if (lstrcmpi(argv[1], TEXT("/stop")) == 0) {
        DoStopSvc();

        std::cout << "Служба остановлена" << std::endl;
    }
    else if (lstrcmpi(argv[1], TEXT("/uninstall")) == 0) {
        DoDeleteSvc();

        std::cout << "Служба удалена" << std::endl;
    }
    else if (lstrcmpi(argv[1], TEXT("/service")) == 0) {
        // Main work service
        SERVICE_TABLE_ENTRYW DispatchTable[] =
        {
            { WITHOUT_CONST_W(SVCNAME), (LPSERVICE_MAIN_FUNCTIONW)SvcMain},
            { NULL, NULL }
        };

        if (!StartServiceCtrlDispatcherW(DispatchTable))
        {
            DWORD lastError = GetLastError();

            switch (lastError) {
            case ERROR_FAILED_SERVICE_CONTROLLER_CONNECT: {
                loggerError << "ERROR_FAILED_SERVICE_CONTROLLER_CONNECT";
                break;
            }
            case ERROR_INVALID_DATA: {
                loggerError << "ERROR_INVALID_DATA";
                break;
            }
            case ERROR_SERVICE_ALREADY_RUNNING: {
                loggerError << "ERROR_SERVICE_ALREADY_RUNNING";
                break;
            }
            }

            //  SvcReportEvent(WITHOUT_CONST(TEXT("StartServiceCtrlDispatcher")));
            return;
        }
    }
    else {
        DoStopSvc();
        std::cout << "Служба остановлена" << std::endl;

        DoDeleteSvc();
        std::cout << "Служба удалена" << std::endl;

        SvcInstall();
        std::cout << "Служба установлена" << std::endl;

        DoStartSvc();
        std::cout << "Служба запущена" << std::endl;
    }
}

// Define main function
#define MAIN_FUNC int __cdecl _tmain(int argc, TCHAR* argv[])
#endif

std::atomic<bool> running;

/* Close terminal handler */
BOOL WINAPI ConsoleHandleRoutine(DWORD dwCtrlType) {
    switch (dwCtrlType) {
    case CTRL_C_EVENT:
    case CTRL_CLOSE_EVENT:
    case CTRL_LOGOFF_EVENT:
    case CTRL_SHUTDOWN_EVENT: {
        logger << (LogMsg() << "Закрытие терминала");
        logger << (LogMsg() << "[Информация о процессе] (ConsoleHandleRoutine): PID = " << GetCurrentProcessId() << "; Thread ID = " << std::this_thread::get_id());

        running.store(false);

        break;
    }
    default: {
        break;
    }
    }

    return TRUE;
}

void MemoryMonitoring() {
    while (running.load()) {
        logger << (LogMsg() << "[Мониторинг ресурсов системы] Используемая приложением память: " << getMemoryCurrentProcessInfo()
            << "; Количество потоков: " << get_current_thread_count());

        std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    }
}

bool processArgument(std::string& arg, std::string& bodyArg) {
    if (arg == "--help") {
        std::cout << "\nArgument --ip - this is IP address server" << std::endl;
        std::cout << "Argument --port - this is port server" << std::endl;
        std::cout << "\nExamples: " << std::endl << std::endl;
        std::cout << "prt_stat.exe --ip=\"10.0.1.199\" --port=\"1111\"" << std::endl;
        std::cout << "prt_stat.exe --port=\"1111\" --ip=\"10.0.1.199\"" << std::endl;
        std::cout << "prt_stat.exe --ip=\"10.0.1.199\"" << std::endl;
        std::cout << "prt_stat.exe --port=\"1111\"" << std::endl << std::endl;

        std::cout << "Default --ip is 127.0.0.1" << std::endl;
        std::cout << "Default --port is 1111" << std::endl;

        return false;
    }
    else if (arg == "--ip") {
        std::string body = std::string(bodyArg);
        std::vector<std::string> vec = split(body, ".");

        if (vec.size() != 4) {
            std::cout << "Incorrect IP server: " << body;
            return false;
        }

        for (size_t i = 0; i < vec.size(); i++) {
            if (!is_number(vec[i])) {
                std::cout << "Incorrect IP server: " << body;
                return false;
            }
            else {
                int number = std::stoi(vec[i]);
                if (number < 0 || number > 255) {
                    std::cout << "Incorrect IP server: " << body;
                    return false;
                }
            }
        }
    }
    else if (arg == "--port") {
        std::string body = std::string(bodyArg);
        if (!is_number(body)) {
            std::cout << "Incorrect port: " << body;
            return false;
        }

        if (std::stoi(body) < 0) {
            std::cout << "Incorrect port: " << body;
            return false;
        }
    }
    else {
        std::cout << "Argument '" << arg << "' is not supported. Please see help (for do it start app with --help argument)";

        return false;
    }

    return true;
}

// Entry point
int main(int argc, char* argv[])
{
    setlocale(LC_ALL, "ru");

    bool writeToConsole = false;
#ifdef WIN_OS
    writeToConsole = console_utils::consoleExists();
#endif

    try {
        // Update values in loggers
        logger.loadEnvValues();
        loggerError.loadEnvValues();

        ConfEnv::loadConfigEnv(argc <= 1);

        if (argc <= 1) {
            logger << (LogMsg() << "[Информация о процессе]: PID = " << GetCurrentProcessId() << "; Thread ID = " << std::this_thread::get_id());
        }

#if defined(DEBUG) && (DEBUG == 0)
        writeToConsole = false;

        // Calling the main function
        mainFunc(argc, argv);
#endif

    }
    catch (std::string e) {
#ifdef WIN_OS
        writeToConsole = console_utils::consoleExists();
#endif

        loggerError << (LogMsg(writeToConsole) << e);
    }
    catch (const ConfEnvException& e) {
#ifdef WIN_OS
        writeToConsole = console_utils::consoleExists();
#endif

        loggerError << (LogMsg(writeToConsole) << e.what());
    }

    return 0;
}

#ifdef WIN_OS

/// <summary>
/// Entry point for the service
/// </summary>
/// <param name="dwArgc">Number of arguments in the lpszArgv array</param>
/// <param name="lpszArgv">Array of strings. The first string is the name of
/// the service and subsequent strings are passed by the process
/// that called the StartService function to start the service.</param>
/// <returns>None</returns>
VOID WINAPI SvcMain(DWORD dwArgc, LPWSTR* lpszArgv)
{
    // Register the handler function for the service
    gSvcStatusHandle = RegisterServiceCtrlHandlerW(
        SVCNAME,
        SvcCtrlHandler);

    if (!gSvcStatusHandle)
    {
        loggerError << (LogMsg() << "Нет зарегистрированного обработчика для контроллера (" << GetLastError() << ")");
        SvcReportEvent(WITHOUT_CONST_W(L"RegisterServiceCtrlHandler"));
        return;
    }

    // These SERVICE_STATUS members remain as set here
    gSvcStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    gSvcStatus.dwServiceSpecificExitCode = 0;

    logger << (LogMsg() << "Служба стартует через 3000 миллисекунд");

    // Report initial status to the SCM (set begin state)
    ReportSvcStatus(SERVICE_START_PENDING, NO_ERROR, 3000);

    logger << (LogMsg() << "Запуск службы ...");

    // Perform service-specific initialization and work.
    SvcInit(dwArgc, lpszArgv);
}

/// <summary>
/// The service code
/// </summary>
/// <param name="dwArgc">Number of arguments in the lpszArgv array</param>
/// <param name="lpszArgv">Array of strings. The first string is the name of
/// the service and subsequent strings are passed by the process
/// that called the StartService function to start the service.</param>
/// <returns>None</returns>
VOID SvcInit(DWORD dwArgc, LPWSTR* lpszArgv)
{
    try {
        // Declare and set any required variables.
        // Be sure to periodically call ReportSvcStatus() with 
        // SERVICE_START_PENDING. If initialization fails, call
        // ReportSvcStatus with SERVICE_STOPPED.

        // Create an event. The control handler function, SvcCtrlHandler,
        // signals this event when it receives the stop control code.

        ghSvcStopEvent = CreateEventW(
            NULL,    // default security attributes
            TRUE,    // manual reset event
            FALSE,   // not signaled
            NULL);   // no name

        if (ghSvcStopEvent == NULL)
        {
            DWORD lastError = GetLastError();

            loggerError << (LogMsg() << "Create event failed (" << lastError << ")");
            ReportSvcStatus(SERVICE_STOPPED, GetLastError(), 0);
            return;
        }

        // Report running status when initialization is complete.
        ReportSvcStatus(SERVICE_RUNNING, NO_ERROR, 0);
        logger << (LogMsg() << "Служба выполняется");

        // Load and build config
        ConfEnv::loadConfigEnv();

        // Monitoring configuration file changes
        FileWatcher fw(CONFIG_INI_PATH, []() {
            logger.endl();
            logger << (LogMsg() << "Начало обработки изменений в конфигурационном файле (" << CONFIG_INI_PATH << ")");

            // Load and build config
            ConfEnv::loadConfigEnv();

            logger << (LogMsg() << "Завершение обработки изменений в конфигурационном файле (" << CONFIG_INI_PATH << ")");
            logger.endl();
            });

        std::uint64_t iter = 1;

        // Perform work until service stops.
        while (WaitForSingleObject(ghSvcStopEvent, 0) != WAIT_OBJECT_0)
        {
            if (iter == UINT64_MAX) {
                iter = 1;
            }

            logger << (LogMsg() << "Итерация " << iter++);

            Sleep(2000);
        }

        fw.release();

        logger << (LogMsg() << "Служба остановлена");
        ReportSvcStatus(SERVICE_STOPPED, NO_ERROR, 0);
    }
    catch (const ConfEnvException& ec) {
        logger << (LogMsg() << "Служба аварийно остановлена");
        loggerError << (LogMsg() << ec.what());
        ReportSvcStatus(SERVICE_STOPPED, NO_ERROR, 0);
    }
}

#endif