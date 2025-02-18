#pragma once

#include <exception>
#include <string>

class ConfEnvException : public std::exception {
private:
	std::string message;

public:
	ConfEnvException(const std::string&);

public:
	const char* what() const throw();
};