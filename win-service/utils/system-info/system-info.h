#pragma once

#include <string>
#include <vector>
#include <filesystem>
#include "../../os.h"
#include "../utils.h"

#ifdef WIN_OS
#include <windows.h>
#include <direct.h>
#endif

#ifdef LINUX_OS
#include <unistd.h>
#include <limits.h>
#endif

std::string getMyComputerName();
std::string getExecutableFilepath();
std::string getWorkingDirectory();