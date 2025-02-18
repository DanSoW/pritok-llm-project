#pragma once

#include <chrono>
#include <ctime>

#include <filesystem>
#include <fstream>

template<typename T>
using tp_sys_clock = std::chrono::time_point<std::chrono::system_clock, T>;

template<typename T = std::chrono::days, std::enable_if_t<std::chrono::_Is_duration_v<T>, int> = 0>
tp_sys_clock<T> dateToTP_SysClock(int year = 1900, int month = 1, int day = 1, int hour = 0, int minute = 0, int second = 0) {
    std::tm data;

    data.tm_sec = second;
    data.tm_min = minute;
    data.tm_hour = hour;
    data.tm_mday = day;
    data.tm_mon = (month - 1);
    data.tm_year = (year - 1900);

    return std::chrono::floor<T>(
        std::chrono::system_clock::from_time_t(_mkgmtime(&data))
    );
}

std::time_t dateTimeToTimeT(int year = 1900, int month = 1, int day = 1, int hour = 0, int minute = 0, int second = 0);
std::time_t dateToTimeT(const std::string& date, bool is_dst = false, const std::string& format = "%d.%m.%Y");

template <typename TP>
std::time_t to_time_t(TP tp)
{
    using namespace std::chrono;
    auto sctp = time_point_cast<system_clock::duration>(tp - TP::clock::now()
        + system_clock::now());
    return system_clock::to_time_t(sctp);
}

std::time_t file_last_write_time_t(const std::filesystem::path&);