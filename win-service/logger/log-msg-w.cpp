#include "log-msg-w.h"

LogMsgW::LogMsgW(const bool& to_console) : _message{ L"" }, _to_console{ to_console } {}
LogMsgW::LogMsgW(const std::wstring& msg, const bool& to_console) : _message{ msg }, _to_console{ to_console } {}

std::wstring LogMsgW::operator() () {
	return this->_message;
}

std::wstring LogMsgW::getMessage() {
	return this->_message;
}

bool LogMsgW::toConsole() {
	return this->_to_console;
}