#pragma once

#include <string>
#include <sstream>
#include "../utils/string-processing/string-processing.h"

namespace str_proc = string_processing;

/* Class message for logger */
class LogMsgW {
private:
	std::wstring _message;
	bool _to_console;

public:
	LogMsgW(const bool& to_console = false);
	LogMsgW(const std::wstring& message, const bool& to_console = false);

public:
	std::wstring operator() ();

	template<typename T>
	LogMsgW& operator<<(T value) {
		std::wstringstream stream;
		stream << value;

		this->_message += stream.str();
		return *this;
	}

	template<>
	LogMsgW& operator<<(const std::string& value) {
		std::wstringstream stream;
		stream << str_proc::stringToWstring(value);

		this->_message += stream.str();
		return *this;
	}

	template<>
	LogMsgW& operator<<(const char* value) {
		std::wstringstream stream;
		stream << str_proc::stringToWstring(value);

		this->_message += stream.str();
		return *this;
	}

public:
	std::wstring getMessage();
	bool toConsole();
};