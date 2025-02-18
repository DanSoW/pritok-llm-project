#pragma once

#include "../../os.h"

#ifdef WIN_OS
#define _WINSOCKAPI_
#include "../../logger/logger.h"
#include <windows.h>

namespace console_utils {
	void blockQuickChange();
	int blockScroll();
	int blockResize();
	bool consoleExists();
}

#endif