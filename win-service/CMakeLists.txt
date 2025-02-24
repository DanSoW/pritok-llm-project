cmake_minimum_required (VERSION 3.8)

# Включение горячей перезагрузки для компиляторов MSVC, если поддерживается.
if (POLICY CMP0141)
    cmake_policy(SET CMP0141 NEW)
    set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("win-service")

# Установка флага DEBUG
set (DEBUG 0)

set (CMAKE_CXX_STANDARD 20)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++20 /Zc:__cplusplus")
endif()

set (CMAKE_C_STANDARD 17)
set (CMAKE_C_STANDARD_REQUIRED ON)

# Add configurable variables
set (APP_NAME "win-service")
set (APP_VERSION "debug")
set (APP_PACKAGE "win-service")

set (FILE_NAME_LOG "app")
set (FILE_NAME_LOG_ERROR "err")

# Сепаратор пути для Windows
set (SEP_WIN "\\\\")

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set (DIR_SEPARATOR "'${SEP_WIN}'")
    set (CONFIG_INI_PATH "${CMAKE_SOURCE_DIR}${SEP_WIN}@win-service.ini")
elseif (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set (DIR_SEPARATOR '/')
else()
    set (DIR_SEPARATOR '/')
endif()

# Определение конфигурационного файла
configure_file (config.h.in ${CMAKE_CURRENT_SOURCE_DIR}/config.h @ONLY)

find_package(Python3 COMPONENTS Development REQUIRED)

# Файлы проекта
add_executable (win-service "win-service.cpp" "win-service.h" "logger/logger.h" "logger/logger.cpp" "utils/utils.h" "config.h" "utils/utils.cpp" "exceptions/LogException.h" "exceptions/LogException.cpp" "logger/log-msg.h" "logger/log-msg.cpp" "net/IPAddress.h" "net/IPAddress.cpp" "utils/system-info/system-info.h" "utils/system-info/system-info.cpp" "utils/types.h" "utils/string-processing/string-processing.h" "utils/string-processing/string-processing.cpp" "logger/log-level.h" "logger/log-level.cpp" "logger/log-msg-w.cpp" "logger/log-msg-w.h" "conf-env/conf-env.h" "conf-env/conf-env.cpp" "exceptions/ConfEnvException.h" "exceptions/ConfEnvException.cpp" "os.h" "conf-ini/conf-ini.h" "packages/simpleini/ConvertUTF.h" "packages/simpleini/SimpleIni.h" "conf-ini/conf-ini.cpp" "packages/datetime/datetime.h" "packages/datetime/datetime.cpp" "conf-env/conf-keys.h" "conf-env/conf-keys.cpp" "service/win-service-manager.h" "service/win-service-manager.cpp" "utils/process-info/process-info.h" "utils/process-info/process-info.cpp" "utils/memory-control/memory-control.h" "utils/memory-control/memory-control.cpp" "utils/console/console.h" "utils/console/console.cpp" "packages/file-watcher/file-watcher.h" "packages/file-watcher/file-watcher.cpp")

# Исправляет ошибку, которая возникает при сборке релизной версии (связана с constexpr у конструктора mutex в релизной сборке)
add_compile_definitions(_DISABLE_CONSTEXPR_MUTEX_CONSTRUCTOR)

# Подключение библиотеки Python
target_link_libraries(${PROJECT_NAME} Python3::Python)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET win-service PROPERTY CXX_STANDARD 20)
endif()
