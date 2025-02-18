#include "datetime.h"

std::time_t dateTimeToTimeT(int year, int month, int day, int hour, int minute, int second) {
    std::tm data;

    data.tm_sec = second;
    data.tm_min = minute;
    data.tm_hour = hour;
    data.tm_mday = day;
    data.tm_mon = (month - 1);
    data.tm_year = (year - 1900);

    auto date = std::chrono::system_clock::from_time_t(_mkgmtime(&data)).time_since_epoch();

    time_t seconds = time_t(std::chrono::duration_cast<std::chrono::seconds>(date).count());
    /*auto localField = *std::gmtime(&seconds);
    time_t secondsUTC = mktime(&localField);*/

    return seconds;
}

std::time_t dateToTimeT(const std::string& date, bool is_dst, const std::string& format) {
    std::tm t = { 0 };

    t.tm_isdst = is_dst ? 1 : 0;

    std::istringstream ss(date);
    ss >> std::get_time(&t, format.c_str());

    return mktime(&t);
}

std::time_t file_last_write_time_t(const std::filesystem::path& target) {
    auto fileTime = std::filesystem::last_write_time(target);
    auto systemTime = std::chrono::clock_cast<std::chrono::system_clock>(fileTime);
    auto time = std::chrono::system_clock::to_time_t(systemTime);

    return time;
}