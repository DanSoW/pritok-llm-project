#pragma once

#include "../config.h"
#include "../conf-env/conf-env.h"

#include <iostream>
#include <string>
#include <fstream>
#include <ctime>
#include <mutex>
#include <codecvt>
#include "./log-msg.h"
#include "./log-msg-w.h"
#include "./log-level.h"
#include "../utils/system-info/system-info.h"
#include "../utils/string-processing/string-processing.h"

namespace str_proc = string_processing;

/* Base class for logger */
class Logger {
private:
	const std::string title;
	std::string path;
	LogLevel level;

	// Maximum size file for writing logs
	std::int64_t maxSize = ConfEnv::MAX_LOG_SIZE * 1024 * 1024;

	std::string filename = "";
	std::wfstream fileout;
	std::time_t date = time(nullptr);

	// Part of the common log
	std::uint16_t part = 0;

	// Mutex for thread security
	std::mutex mtx;

private:
	std::unique_ptr<std::codecvt_utf8<wchar_t>> codec;
	std::locale utf8Locale;

public:
	Logger(std::string title, std::string path, LogLevel level = LogLevel::Debug);
	virtual ~Logger();

public:
	bool writeData(std::string text, std::string comment = "", bool to_console = false);
	bool writeData(std::wstring text, std::wstring comment = L"", bool to_console = false);
	void write(std::string line);
	void setPath(std::string path);
	void setMaxSize(std::int64_t);
	void setLogLevel(const LogLevel&);
	void endl();
	void endl(const std::uint32_t&);

public:
	Logger& operator() (const LogLevel&);
	Logger& operator<<(const std::string&);
	Logger& operator<<(LogMsg&);
	Logger& operator<<(LogMsgW&);

private:
	bool prepareFile();

public:
	void loadEnvValues();
};

/* Define singleton for Logger */
template <LogLevel Type>
class LoggerSingleton {
public:
	static Logger& GetInstance(const std::string& title, const std::string& path) {
		static Logger instance = Logger(title, path, Type);
		return instance;
	}

	LoggerSingleton() = delete;
	LoggerSingleton(const LoggerSingleton&) = delete;
	const LoggerSingleton& operator=(const LoggerSingleton&) = delete;
};

// Defined logger
extern Logger& logger;
extern Logger& loggerError;