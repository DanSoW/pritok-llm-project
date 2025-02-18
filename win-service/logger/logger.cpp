#include "../net/IPAddress.h"

#include "logger.h"
#include <filesystem>
#include <algorithm>
#include "../utils/utils.h"
#include "../exceptions/LogException.h"
#include "../os.h"
#include <climits>

namespace fs = std::filesystem;

#if defined(LINUX_OS)
#define DIRUMASK fs::perms::owner_read | fs::perms::owner_write | fs::perms::owner_exec \
	| fs::perms::group_read | fs::perms::group_write | fs::perms::group_exec \
	| fs::perms::others_read | fs::perms::others_write | fs::perms::others_exec
#define LOGUMASK fs::perms::owner_read | fs::perms::owner_write \
	| fs::perms::group_read | fs::perms::group_write \
	| fs::perms::others_read | fs::perms::others_write
#endif

Logger::Logger(std::string title, std::string path, LogLevel level) :
	title{ title }, path{ path }, level{ level }
{
	this->codec = std::make_unique<std::codecvt_utf8<wchar_t>>();

	if ((codec != nullptr) && (codec.get() != nullptr)) {
		this->utf8Locale = std::locale(std::locale(), this->codec.get());
	}
}

Logger::~Logger() {
	if (this->fileout.is_open()) {
		this->fileout.close();
	}
}

/* Set new logger path */
void Logger::setPath(std::string path) {
	// std::lock_guard<std::mutex> lock(this->mtx);

	if (this->fileout.is_open()) {
		this->fileout.close();
	}

	this->filename = "";
	this->path = path;
}

/* Set maximum size logger */
void Logger::setMaxSize(std::int64_t size) {
	// std::lock_guard<std::mutex> lock(this->mtx);
	this->maxSize = size * 1024 * 1024;
}

/* Change log level */
void Logger::setLogLevel(const LogLevel& level) {
	// std::lock_guard<std::mutex> lock(this->mtx);
	this->level = level;
}

/* Write data in logger */
bool Logger::writeData(std::string text, std::string comment, bool to_console) {
	// Block mutex
	std::lock_guard<std::mutex> lock(this->mtx);

#if defined(DEBUG) && (DEBUG == 1)
	std::cout << text << std::endl;
#endif

	if (to_console) {
		std::cout << text << std::endl;
	}

	try {
		if (this->prepareFile()) {
			this->fileout << str_proc::stringToWstring(timePointToString<double, 3>(std::chrono::system_clock::now())) << L" ["
				<< str_proc::stringToWstring(LogLevelHelper::to_string(this->level)) << L"]:" << L" "
				<< str_proc::stringToWstring(text) << str_proc::stringToWstring(comment) << std::endl;
			return true;
		}
		else {
			throw LogException("Возникла непредвиденная ошибка при обработке файла: ", this->filename);
		}
	}
	catch (LogException err) {
		std::cout << err.what() << std::endl;
	}

	return false;
}

bool Logger::writeData(std::wstring text, std::wstring comment, bool to_console) {
	// Block mutex
	std::lock_guard<std::mutex> lock(this->mtx);

#if defined(DEBUG) && (DEBUG == 1)
	std::wcout << text << std::endl;
#endif

	if (to_console) {
		std::wcout << text << std::endl;
	}

	try {
		if (this->prepareFile()) {
			this->fileout << str_proc::stringToWstring(timePointToString<double, 3>(std::chrono::system_clock::now())) << L" ["
				<< str_proc::stringToWstring(LogLevelHelper::to_string(this->level)) << L"]:" << L" "
				<< text << comment << std::endl;
			return true;
		}
		else {
			throw LogException("Возникла непредвиденная ошибка при обработке файла: ", this->filename);
		}
	}
	catch (LogException err) {
		std::cout << err.what() << std::endl;
	}

	return false;
}

/* Write data in log file */
void Logger::write(std::string line) {
	this->writeData(line);
}

void Logger::endl() {
	this->write("");
}

void Logger::endl(const std::uint32_t& count) {
	for (std::uint32_t i = 0; i < count; i++) {
		this->write("");
	}
}

Logger& Logger::operator()(const LogLevel& level) {
	// std::lock_guard<std::mutex> lock(this->mtx);
	this->setLogLevel(level);

	return *this;
}

Logger& Logger::operator<<(const std::string& line) {
	this->writeData(line);

	return *this;
}

Logger& Logger::operator<<(LogMsg& msg) {
	this->writeData(msg(), "", msg.toConsole());

	return *this;
}

Logger& Logger::operator<<(LogMsgW& msg) {
	this->writeData(msg(), L"", msg.toConsole());

	return *this;
}

/* Preparing and opening a file */
bool Logger::prepareFile() {
	try {
		bool b = false;

		time_t current = time(nullptr);
		struct tm dateTm = *localtime(&this->date);
		struct tm dateTmCurrent = *localtime(&current);

		if (!(dateEqual(dateTm, dateTmCurrent))) {
			this->date = current;
			this->part = 0;
			b = true;

			dateTm = dateTmCurrent;
		}

		if ((!b) && !this->filename.empty()
			&& std::filesystem::exists(this->filename)
			&& (fs::file_size(std::filesystem::path(this->filename)) > this->maxSize)) {
			b = true;

			if (++this->part == USHRT_MAX) {
				return false;
			}
		}

		b = b || !this->fileout.is_open();

		if (b) {
			if (this->fileout.is_open()) {
				this->fileout.close();
			}

			std::string partStr = (this->part > 0) ? "~" + std::to_string(this->part) : "";
			std::string dateStr = dateToString(dateTm);

			this->filename = this->path + DIR_SEPARATOR
				+ dateStr + DIR_SEPARATOR
				+ APP_NAME + "-" + APP_VERSION + DIR_SEPARATOR
				+ partStr + this->title + "_" + dateStr + ".log";

			std::vector<std::filesystem::path> dirs = {
				this->path,
				this->path + DIR_SEPARATOR + dateStr,
				this->path + DIR_SEPARATOR + dateStr + DIR_SEPARATOR + APP_NAME + "-" + APP_VERSION
			};

			std::error_code ec;
			for (const auto& i : dirs) {
				if (!std::filesystem::exists(i, ec) && !ec) {
					std::filesystem::create_directory(i, ec);
				}
				else if (!ec) {
					if (!std::filesystem::is_directory(i, ec) && !ec) {
						std::filesystem::create_directory(i, ec);

						if (!ec) {
#ifdef defined(LINUX_OS)
							fs::permissions(i, DIRUMASK, ec);
#endif
						}
					}
				}

				if (ec) {
					std::cout << "Ошибка при создании директории: " << ec.message() << std::endl;
					return false;
				}
			}

			this->fileout.open(this->filename, std::ios::app | std::ios::ate);

			if (this->codec != nullptr && this->codec.get() != nullptr) {
				fileout.imbue(this->utf8Locale);
			}

			/* Mark log file only when it is empty */
			if (fs::file_size(std::filesystem::path(this->filename)) == 0) {
				IPv4 ip;
				getMyIP(ip);

				std::string appInfo = "[-]User@" + getMyComputerName() + "(" + convertIPtoString(ip) + ")";

				this->fileout << L"/*" << std::endl
					<< L" * " << str_proc::stringToWstring(std::string(APP_NAME)) << L"/" << str_proc::stringToWstring(std::string(APP_VERSION)) << std::endl
					<< L" * " << str_proc::stringToWstring(appInfo) << std::endl
					<< L" * " << str_proc::stringToWstring(std::string(APP_PACKAGE)) << std::endl
					<< L" * " << str_proc::stringToWstring(currentDateTime()) << std::endl
					<< L" */" << std::endl << std::endl;
			}

			if (this->fileout.is_open()) {
				// this->fileout << std::endl;

#ifdef LINUX_OS
				fs::permissions(std::filesystem::path(this->filename), LOGUMASK);
#endif

				return true;
			}

			return false;
		}

		return true;
	}
	catch (std::filesystem::filesystem_error er) {
		std::cout << er.what() << std::endl;
		return false;
	}
}

void Logger::loadEnvValues() {
	this->setPath(ConfEnv::PATH_LOGS);
	this->setMaxSize(ConfEnv::MAX_LOG_SIZE);
}

Logger& logger = LoggerSingleton<LogLevel::Debug>::GetInstance(FILE_NAME_LOG, ConfEnv::PATH_LOGS);
Logger& loggerError = LoggerSingleton<LogLevel::Error>::GetInstance(FILE_NAME_LOG_ERROR, ConfEnv::PATH_LOGS);