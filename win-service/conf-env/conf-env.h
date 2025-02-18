#pragma once

#include <cerrno>
#include <string>
#include "../config.h"
#include "../conf-ini/conf-ini.h"
#include "../utils/system-info/system-info.h"
#include "../exceptions/ConfEnvException.h"
#include "conf-keys.h"

template<typename T>
concept ConfEnvValue = std::is_integral_v<T> || std::is_floating_point_v<T>;

#define CE_INT_NUMBERS "0123456789"
#define CE_FLOAT_NUMBERS "0123456789."

class ConfEnv {
public:
	static std::string WORKING_DIRECTORY;

	static std::string PATH_LOGS;
	static int MAX_LOG_SIZE;

public:
	static void loadConfigEnv(bool = false);

private:
	static void loadItem(const char*, const char*, std::string&, bool = true);

	/* Loading data with an integer or a real number */
	template<ConfEnvValue T>
	static void loadItemByNumber(const char* section, const char* key, T& item) {
		if (confIni.KeyExists(section, key)) {
			std::string value = confIni.GetValue(section, key, "");

			if (value.size() > 0) {
				if (std::is_integral_v<T> && (value.find_first_not_of(CE_INT_NUMBERS) == std::string::npos)) {
					item = std::stoi(value);
				}
				else {
					int cvalue = std::count(value.begin(), value.end(), '.');

					if ((cvalue <= 1) && (value.find_first_not_of(CE_FLOAT_NUMBERS) == std::string::npos)) {
						item = std::stof(value);
					}
				}
			}
		}
	}
};