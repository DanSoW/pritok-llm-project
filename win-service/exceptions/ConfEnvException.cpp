#include "ConfEnvException.h"

ConfEnvException::ConfEnvException(const std::string& msg) :
	message{ msg } {}

const char* ConfEnvException::what() const throw() {
	return message.c_str();
}