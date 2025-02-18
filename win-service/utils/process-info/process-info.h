#pragma once

#include "../../os.h"

#ifdef WIN_OS
#include <Windows.h>
#include <TlHelp32.h>

int get_current_thread_count();

#endif