#pragma once

#if defined(_WIN32) || defined(_WIN64)
#define WIN_OS
#endif


#if defined(__linux__)
#define LINUX_OS

#elif defined(__unix__) || defined(__unix) || defined(unix)
#define UNIX_OS

#elif defined(_WIN32) || defined(WIN32)
#define WIN32_OS

#elif defined(_WIN64) || defined(WIN64)
#define WIN64_OS

#endif