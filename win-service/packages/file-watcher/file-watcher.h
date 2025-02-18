#pragma once

#include <functional>
#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <utility>
#include <vector>
#include <array>
#include <unordered_map>
#include <unordered_set>
#include <system_error>
#include <string>
#include <algorithm>
#include <type_traits>
#include <future>
#include <regex>
#include <cstddef>
#include <cstring>
#include <cwchar>
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <filesystem>
#include "../../logger/logger.h"
#include "../datetime/datetime.h"

namespace fs = std::filesystem;

class FileWatcher {
private:
	std::string _filepath;
	std::function<void()> _callback;

	std::mutex _callback_mutex;
	std::thread _callback_thread;

	std::promise<void> _running;
	std::atomic<bool> _destroy{ false };

public:
	FileWatcher(const std::string&, const std::function<void()>&);
	~FileWatcher();

private:
	void init();
	void destroy();

	void callback_thread();

public:
	void release();
};