#pragma once

#include <string>
#include <type_traits>
#include <iomanip>
#include <sstream>
#include <cmath>
#include <algorithm>
#include "../utils.h"

namespace string_processing {
	std::string left(std::string str, std::size_t n);
	std::string right(std::string str, std::size_t n);
	std::wstring stringToWstring(const char*& str);
	std::wstring stringToWstring(const std::string& str);
	bool is_number(const std::string& s);

	template<typename T>
		requires std::is_arithmetic_v<T>
	std::string to_number_precision(T number, std::streamsize precision) {
		if (std::is_floating_point<T>::value) {
			double empty;
			if (std::modf((double)number, &empty) == 0) {
				return std::to_string((long)number);
			}

			std::stringstream stream;
			stream << std::fixed << std::setprecision(precision) << number;

			return stream.str();
		}

		return std::to_string(number);
	}

	template<typename T>
		requires std::is_floating_point_v<T>
	T to_floating_point_value(std::string data) {
		auto itPoint = std::find(data.begin(), data.end(), '.');

		auto processing = [&]() -> T {
			std::vector<char> vec{ *itPoint, '\0' };
			std::string s(vec.data());

			vec.clear();

			std::vector<std::string> parts = split(data, s);
			T result = 0.0;

			if (!is_number(parts.at(0)) || !is_number(parts.at(1))) {
				return result;
			}

			std::uint8_t precision = parts.at(1).size();
			if (precision > 38) {
				precision = 38;
			}

			std::int32_t value1 = std::stoi(parts.at(0));
			std::int32_t value2 = std::stoi(parts.at(1));

			std::int32_t count = pow(10, precision);

			result += value1;
			result += ((T)value2 / (T)count);

			return result;
			};

		if (itPoint != data.end()) {
			return processing();
		}
		else {
			itPoint = std::find(data.begin(), data.end(), ',');

			if (itPoint != data.end()) {
				return processing();
			}
		}

		return 0.0;
	}

	template<typename T>
	std::string valueToString(T value) {
		std::string str = "";

		std::stringstream stream;
		stream << value;

		str += stream.str();
		return str;
	}

	std::string& leftTrim(std::string&, const std::string& chars);
	std::string& rightTrim(std::string&, const std::string& chars);
	std::string& trimString(std::string&, const std::string& chars);

	std::wstring& leftTrimW(std::wstring&, const std::wstring& chars);
	std::wstring& rightTrimW(std::wstring&, const std::wstring& chars);
	std::wstring& trimStringW(std::wstring&, const std::wstring& chars);

	template<typename T, typename D>
		requires std::is_convertible_v<T, D>
	void trimVector(std::vector<T>& vec, T value, D zero) {
		const auto isNonZero = [&](T item) {
			return (D)item != zero;
			};

		const auto firstNonZero = std::find_if(vec.begin(), vec.end(), isNonZero);
		vec.erase(vec.begin(), firstNonZero);

		const auto lastNonZero = std::find_if(vec.rbegin(), vec.rend(), isNonZero);
		vec.erase(lastNonZero.base(), vec.end());
	}
}