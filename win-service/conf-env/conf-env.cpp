#include "conf-env.h"
#include "../logger/logger.h"

#include "../packages/datetime/datetime.h"

// Filepath to working directory
std::string ConfEnv::WORKING_DIRECTORY{ getWorkingDirectory() };

// Filepath to logs
std::string ConfEnv::PATH_LOGS{ ConfEnv::WORKING_DIRECTORY };

// Max logs size
int ConfEnv::MAX_LOG_SIZE{ 1000 };

void ConfEnv::loadConfigEnv(bool log) {
	if (!std::filesystem::exists(CONFIG_INI_PATH)) {
		throw ConfEnvException("Ошибка: конфигурационного файла по пути \"" + std::string(CONFIG_INI_PATH) + "\" не обнаружено");
	}

	// File descriptor (handle)
	FILE* fp = NULL;

	auto lambdaOpen = [&]() {
#if __STDC_WANT_SECURE_LIB__ && !_WIN32_WCE
		fopen_s(&fp, CONFIG_INI_PATH, "rb+");
#else // !__STDC_WANT_SECURE_LIB__
		fp = fopen(PRITOK_INI_PATH, "rb+");
#endif // __STDC_WANT_SECURE_LIB__
		};

	lambdaOpen();

	std::uint16_t count = 1;
	while (!fp && count < 60) {
		loggerError << (LogMsg() << "Ошибка при захвате файла конфигурации (" << CONFIG_INI_PATH << "): " << std::strerror(errno) << "; Попытка №" << count++);
		std::this_thread::sleep_for(std::chrono::seconds(1));

		lambdaOpen();
	}

	if (!fp) {
		throw ConfEnvException("Ошибка: не удалось захватить конфигурационный файл спустя 60 попыток");
	}

	// Reset old data
	confIni.Reset();
	SI_Error err = confIni.LoadFile(fp);

	if (err != SI_OK) {
		loggerError << (LogMsg() << "Ошибка при загрузке конфигурационного файла (" << CONFIG_INI_PATH << "): " << std::strerror(errno));
		fclose(fp);
	}

	fseek(fp, 0, SEEK_SET);

	if (!confIni.IsEmpty()) {
		auto lambdaWrite = [&]() {
			bool spaces = confIni.UsingSpaces();
			confIni.SetSpaces(false);

			SI_Error errSave = confIni.SaveFile(fp);

			if (errSave != SI_OK) {
				loggerError << (LogMsg() << "Ошибка при загрузке данных в конфигурационный файл (" << CONFIG_INI_PATH << "): " << std::strerror(errno));
			}
			confIni.SetSpaces(spaces);
			};

		/* Load logger values */
		ConfEnv::loadItem(ConfKeys::LOGS_SECTION_KEY, ConfKeys::ALL_LOGS_PATH_VALUE_KEY, ConfEnv::PATH_LOGS);
		ConfEnv::loadItemByNumber(ConfKeys::LOGS_SECTION_KEY, ConfKeys::MAX_LOGS_KEEP_SIZE_VALUE_KEY, ConfEnv::MAX_LOG_SIZE);

		logger.loadEnvValues();
		loggerError.loadEnvValues();

		/* Load port */
		// ...
	}

	// Release resource
	fclose(fp);

	if (log) {
		logger << (LogMsg() << "Конфигурация из конфигурационного файла (" << CONFIG_INI_PATH << ") загружена");
	}
}

void ConfEnv::loadItem(const char* section, const char* key, std::string& item, bool check) {
	if (!check || check && confIni.KeyExists(section, key)) {
		std::string value = confIni.GetValue(section, key, "");

		if (value.size() > 0) {
			item = value;
		}
	}
}