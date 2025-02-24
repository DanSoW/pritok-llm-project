#include "utils.h"

/* Implementation of the split function for strings */
std::vector<std::string> split(const std::string& str, const std::string& delimiters) {
	if (str.size() == 0) {
		return std::vector<std::string>();
	}

	std::vector<std::string> tokens;
	std::string::size_type lastPos = str.find_first_not_of(delimiters, 0);
	std::string::size_type pos = str.find_first_of(delimiters, lastPos);

	while (std::string::npos != pos || std::string::npos != lastPos) {
		tokens.push_back(str.substr(lastPos, pos - lastPos));

		lastPos = str.find_first_not_of(delimiters, pos);
		pos = str.find_first_of(delimiters, lastPos);
	}

	return tokens;
}

std::vector<std::wstring> splitW(const std::wstring& str, const std::wstring& delimiters) {
	if (str.size() == 0) {
		return std::vector<std::wstring>();
	}

	std::vector<std::wstring> tokens;
	std::wstring::size_type lastPos = str.find_first_not_of(delimiters, 0);
	std::wstring::size_type pos = str.find_first_of(delimiters, lastPos);

	while (std::wstring::npos != pos || std::wstring::npos != lastPos) {
		tokens.push_back(str.substr(lastPos, pos - lastPos));

		lastPos = str.find_first_not_of(delimiters, pos);
		pos = str.find_first_of(delimiters, lastPos);
	}

	return tokens;
}

std::string replaceAll(const std::string& str, const std::string& oldStr, const std::string& newStr) {
	std::string copy{ str };
	size_t pos = 0;

	while ((pos = copy.find(oldStr, pos)) != std::string::npos) {
		copy.replace(pos, oldStr.size(), newStr);
		pos += newStr.size();
	}

	return copy;
}

/* Function for comparing two dates */
bool dateEqual(struct tm date1, struct tm date2) {
	bool year = date1.tm_year == date2.tm_year;
	bool month = date1.tm_mon == date2.tm_mon;
	bool day = date1.tm_mday == date2.tm_mday;

	return year && month && day;
}

/* Converting date and time to a string */
std::string dateToString(struct tm date, std::string sep) {
	int year = date.tm_year + 1900;
	int month = date.tm_mon + 1;
	int day = date.tm_mday;

	std::string str = std::to_string(year) + sep;

	if (month < 10) {
		str += "0" + std::to_string(month) + sep;
	}
	else {
		str += std::to_string(month) + sep;
	}

	if (day < 10) {
		str += "0" + std::to_string(day);
	}
	else {
		str += std::to_string(day);
	}

	return str;
}

/* Getting the current date and time */
std::string currentDateTime() {
	time_t     now = time(0);
	struct tm  tstruct;
	char       buf[80];
	tstruct = *localtime(&now);

	strftime(buf, sizeof(buf), "%d.%m.%Y %X", &tstruct);

	return buf;
}

/* Getting the current date */
std::string currentDate() {
	time_t     now = time(0);
	struct tm  tstruct;
	char       buf[80];
	tstruct = *localtime(&now);

	strftime(buf, sizeof(buf), "%d.%m.%Y", &tstruct);

	return buf;
}

/* Getting the current date */
struct tm currentDateTm() {
	time_t     now = time(0);
	struct tm  tstruct;
	char       buf[80];
	tstruct = *localtime(&now);

	return tstruct;
}

std::chrono::system_clock::time_point GFG(const std::string& datetimeString, const std::string& format)
{
	tm tmStruct = {};

	std::istringstream ss(datetimeString);
	ss >> std::get_time(&tmStruct, format.c_str());

	return std::chrono::system_clock::from_time_t(
		mktime(&tmStruct));
}

std::time_t currentDateTimeT() {
	std::string date = currentDate();
	std::chrono::system_clock::time_point parsedTime = GFG(date, "%d.%m.%Y");

	return std::chrono::system_clock::to_time_t(parsedTime);
}

/* Converting a byte array to a string */
std::string byteArrayToString(byte_array bytes) {
	std::string output(bytes.size(), ' ');

	auto modifier = [](char c) {
		return c;
		};

	std::transform(bytes.begin(), bytes.end(), output.begin(), modifier);

	return output;
}

bool isLeapYear(ushort year) {
	return (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0));
}

ushort daysInMonth(uint year, uint month) {
	ushort days;

	if (month == 4 || month == 6 || month == 9 || month == 11) {
		days = 30;
	}
	else if (month == 2) {
		if (isLeapYear(year)) {
			days = 29;
		}
		else {
			days = 28;
		}
	}
	else {
		days = 31;
	}

	return days;
}