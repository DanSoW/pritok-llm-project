#pragma once

#include <iostream>
#include "../packages/simpleini/SimpleIni.h"

/* Class for ini configuration */
class ConfIniSingleton {
public:
	static CSimpleIni& GetInstance(const char* path) throw() {
		static CSimpleIni instance;
		SI_Error rc = instance.LoadFile(path);

		if (rc != SI_OK) {
			std::cout << "Ошибка: не удалось загрузить конфигурационный файл" << std::endl;
		}

		return instance;
	}
	ConfIniSingleton() = delete;
	ConfIniSingleton(const ConfIniSingleton&) = delete;
	const ConfIniSingleton& operator=(const ConfIniSingleton&) = delete;
};

// Export configuration instance
extern CSimpleIni& confIni;