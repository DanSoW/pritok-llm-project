#include "conf-ini.h"
#include "../config.h"

CSimpleIni& confIni = ConfIniSingleton::GetInstance(CONFIG_INI_PATH);