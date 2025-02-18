#include "log-msg.h"

LogMsg::LogMsg(const bool& to_console) : _message{ "" }, _to_console{ to_console } {}
LogMsg::LogMsg(const std::string& msg, const bool& to_console) : _message{ msg }, _to_console{ to_console } {}

std::string LogMsg::operator() () {
	return this->_message;
}

std::string LogMsg::getMessage() {
	return this->_message;
}

bool LogMsg::toConsole() {
	return this->_to_console;
}