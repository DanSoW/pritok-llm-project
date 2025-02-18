#pragma once

#include <numeric>
#include <string>
#include <vector>
#include <map>

/* All log levels */
enum LogLevel : std::uint8_t {
    Info = 0,
    Trace = 1,
    Debug = 3,
    Warning = 4,
    Error = 5,
    Fatal = 6
};

/* Helper for log level */
class LogLevelHelper {
public:
    typedef LogLevel Type;
    static const std::map<Type, std::string> ENUM_STRINGS_MAP;
    static const std::map<std::string, Type> STRINGS_ENUM_MAP;

    /* Convert enum value to string */
    static std::string to_string(Type enumVal)
    {
        auto it = ENUM_STRINGS_MAP.find(enumVal);
        if (it == ENUM_STRINGS_MAP.end()) {
            return std::string("");
        }

        return it->second;
    }

    /* Convert string to enum value */
    static Type from_string(const std::string& value)
    {
        auto it = STRINGS_ENUM_MAP.find(value);
        if (it == STRINGS_ENUM_MAP.end()) {
            return (Type)0;
        }

        return it->second;
    }
};