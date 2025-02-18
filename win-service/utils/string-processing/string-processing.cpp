#include "string-processing.h"
#include <iostream>

namespace string_processing {
	std::string left(std::string str, std::size_t n) {
		std::size_t size = str.size();

		if (!size || size <= n) {
			return str;
		}

		return str.substr(0, n);
	}

	std::string right(std::string str, std::size_t n) {
		std::size_t size = str.size();

		if (!size || size <= n) {
			return str;
		}

		return str.substr((size - n), size);
	}

	// Check string on number
	bool is_number(const std::string& s) {
		if (s.length() == 0) {
			return false;
		}

		std::string::const_iterator it = s.begin();

		while (it != s.end() && std::isdigit(*it)) {
			++it;
		}

		return !s.empty() && (it == s.end());
	}

	std::wstring stringToWstring(const char*& str) {
		std::size_t length = std::mbstowcs(nullptr, str, 0);

		if (length == static_cast<std::size_t>(-1)) {
			std::cerr << "Conversion error" << std::endl;
			return L"";
		}

		std::wstring wstr(length, L'\0');
		std::mbstowcs(&wstr[0], str, length);
	}

	std::wstring stringToWstring(const std::string& str) {
		size_t length = str.length();

		std::wstring wstr(length, L'\0');
		std::mbstowcs(&wstr[0], str.c_str(), length);

		return wstr;
	}

	std::string& leftTrim(std::string& str, const std::string& chars) {
		str.erase(0, str.find_first_not_of(chars));
		return str;
	}
	std::string& rightTrim(std::string& str, const std::string& chars) {
		str.erase(str.find_last_not_of(chars) + 1);
		return str;
	}
	std::string& trimString(std::string& str, const std::string& chars) {
		return leftTrim(rightTrim(str, chars), chars);
	}

	std::wstring& leftTrimW(std::wstring& str, const std::wstring& chars) {
		str.erase(0, str.find_first_not_of(chars));
		return str;
	}
	std::wstring& rightTrimW(std::wstring& str, const std::wstring& chars) {
		str.erase(str.find_last_not_of(chars) + 1);
		return str;
	}
	std::wstring& trimStringW(std::wstring& str, const std::wstring& chars) {
		return leftTrimW(rightTrimW(str, chars), chars);
	}
}