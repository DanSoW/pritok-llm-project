#pragma once

#include <algorithm>
#include <vector>
#include <string>
#include <cstring>
#include <ctime>
#include <chrono>
#include <iomanip>
#include <cmath>
#include <unordered_map>
#include <memory>
#include "types.h"
// #include <zlib.h>

std::vector<std::string> split(const std::string& str, const std::string& delimiters);
std::vector<std::wstring> splitW(const std::wstring& str, const std::wstring& delimiters);

std::string replaceAll(const std::string& str, const std::string& oldStr, const std::string& newStr);

bool dateEqual(struct tm date1, struct tm date2);
std::string dateToString(struct tm date, std::string sep = "");
std::string currentDateTime();
std::string currentDate();
struct tm currentDateTm();
std::chrono::system_clock::time_point GFG(const std::string& datetimeString, const std::string& format);
std::time_t currentDateTimeT();
std::string byteArrayToString(byte_array);

bool isLeapYear(ushort year);
ushort daysInMonth(uint year, uint month);

/* Converting a TimePoint structure to a string of a specific format */
template<
	typename Double = double,
	std::size_t Precision = std::numeric_limits<Double>::digits10,
	typename TimePoint
>
	requires std::is_floating_point_v<Double> && (Precision <= std::numeric_limits<Double>::digits10)
inline std::string timePointToString(const TimePoint& timePoint) throw()
{
	auto seconds = Double(timePoint.time_since_epoch().count())
		* TimePoint::period::num / TimePoint::period::den;
	auto const zeconds = std::modf(seconds, &seconds);

	std::time_t tt(seconds);
	std::ostringstream oss;

	auto const tm = std::localtime(&tt);
	if (!tm) {
		throw std::runtime_error(std::strerror(errno));
	}

	oss << std::put_time(tm, "%H:%M:")
		<< std::setw(Precision + 3) << std::setfill('0')
		<< std::fixed << std::setprecision(Precision)
		<< tm->tm_sec + zeconds;

	if (!oss) {
		throw std::runtime_error("Error converting TimePoint to String");
	};

	return oss.str();
}

/* Converting a string to a TimePoint structure */
template<typename TimePoint>
TimePoint fromString(const std::string& str) throw()
{
	std::istringstream iss{ str };

	std::tm tm{};

	if (!(iss >> std::get_time(&tm, "%Y-%b-%d %H:%M:%S"))) {
		throw std::invalid_argument("get_time");
	}

	TimePoint timePoint{ std::chrono::seconds(std::mktime(&tm)) };

	if (iss.eof()) {
		return timePoint;
	}

	double zz;
	if (iss.peek() != '.' || !(iss >> zz)) {
		throw std::invalid_argument("decimal");
	}

	using hr_clock = std::chrono::high_resolution_clock;
	std::size_t zeconds = zz * hr_clock::period::den / hr_clock::period::num;
	return timePoint += hr_clock::duration(zeconds);
}

template <typename T, typename U>
bool allequal(const T& t, const U& u)
{
	return t == u;
}

template <typename T, typename U, typename... Others>
bool allequal(const T& t, const U& u, Others const &... args)
{
	return (t == u) && allequal(u, args...);
}

template <typename T, typename U>
bool anyequal(const T& t, const U& u)
{
	return t == u;
}

template <typename T, typename U, typename... Others>
bool anyequal(const T& t, const U& u, Others const &... args)
{
	return (t == u) || anyequal(t, args...);
}

template <typename T>
bool assign(T& t1, const T t2)
{
	if (t1 != t2) {
		t1 = t2;

		return true;
	}

	return false;
}

#define BOOL2STR(b) ((b) ? "true" : "false")

template<typename K, typename V>
std::vector<std::pair<K, V>> mapToVector(const std::unordered_map<K, V>& data) {
	return std::vector<std::pair<K, V>>(data.begin(), data.end());
}

template<typename K, typename V>
std::unique_ptr<std::vector<std::pair<K, V>>> mapToVector(const std::unique_ptr<std::unordered_map<K, V>>& data) {
	std::unique_ptr<std::vector<std::pair<K, V>>> ptr{ std::make_unique<std::vector<std::pair<K, V>>>(data->begin(), data->end()) };
	return std::move(ptr);
}
