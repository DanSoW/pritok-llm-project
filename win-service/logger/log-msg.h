#pragma once

#include <string>
#include <sstream>

/* Class message for logger */
class LogMsg {
private:
	std::string _message;
	bool _to_console;

public:
	LogMsg(const bool& to_console = false);
	LogMsg(const std::string& message, const bool& to_console = false);

public:
	std::string operator() ();

	template<typename T>
	LogMsg& operator<<(T value) {
		std::stringstream stream;
		stream << value;

		this->_message += stream.str();
		return *this;
	}

public:
	std::string getMessage();
	bool toConsole();
};