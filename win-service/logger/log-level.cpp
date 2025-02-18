#include "log-level.h"

const std::map<LogLevelHelper::Type, std::string> LogLevelHelper::ENUM_STRINGS_MAP{
	{Type::Debug, "DEBUG"},
	{Type::Error, "ERROR"},
	{Type::Fatal, "FATAL"},
	{Type::Info, "INFO"},
	{Type::Trace, "TRACE"},
	{Type::Warning, "WARNING"}
};

const std::map<std::string, LogLevelHelper::Type> LogLevelHelper::STRINGS_ENUM_MAP{
	{"DEBUG", Type::Debug},
	{"ERROR", Type::Error},
	{"FATAL", Type::Fatal},
	{"INFO", Type::Info},
	{"TRACE", Type::Trace},
	{"WARNING", Type::Warning}
};