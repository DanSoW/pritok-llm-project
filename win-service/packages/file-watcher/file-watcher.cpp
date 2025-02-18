#include "file-watcher.h"

FileWatcher::FileWatcher(const std::string& filepath, const std::function<void()>& callback) :
	_filepath(filepath), _callback(callback) {
	this->init();
}

FileWatcher::~FileWatcher() {
	this->destroy();
}

void FileWatcher::init() {
	this->_callback_thread = std::thread([this]() {
		try {
			callback_thread();
		}
		catch (...) {
			std::exception_ptr exp = std::current_exception();

			try {
				if (exp) {
					_running.set_exception(exp);
					std::rethrow_exception(exp);
				}
			}
			catch (const std::exception& e) {
				loggerError << (LogMsg() << e.what());
			}
			catch (...) {
				loggerError << (LogMsg() << "Произошла непредвиденная ошибка: FileWatcher::init -> callback_thread()");
			}
		}
		});

	std::future<void> f = _running.get_future();

	// Block until the callback thread is up and running
	f.get();
}

void FileWatcher::callback_thread() {
	std::error_code ec;
	fs::file_time_type last_time;

	if (fs::exists(this->_filepath, ec) && !ec) {
		// last_time = file_last_write_time_t(this->_filepath);
		last_time = fs::last_write_time(this->_filepath);
	}
	else if (ec) {
		loggerError << (LogMsg() << "Ошибка (FileWatcher::callback_thread): " << ec.message());
		return;
	}
	else {
		loggerError << (LogMsg() << "Ошибка (FileWatcher::callback_thread): \"" << this->_filepath << "\" ; " << ec.message());
		return;
	}

	// Set running signal
	this->_running.set_value();

	while (!this->_destroy.load()) {
		std::unique_lock<std::mutex> lock(this->_callback_mutex);

		if (fs::exists(this->_filepath, ec) && !ec) {
			if (last_time != fs::last_write_time(this->_filepath) && this->_callback) {
				try {
					this->_callback();
					std::this_thread::sleep_for(std::chrono::milliseconds(1));
					last_time = fs::last_write_time(this->_filepath);
				}
				catch (const std::exception& e) {
					loggerError << (LogMsg() << "Ошибка (FileWatcher::callback_thread): " << e.what());
					break;
				}
			}
		}
		else if (ec) {
			loggerError << (LogMsg() << "Ошибка (FileWatcher::callback_thread): " << ec.message());
			break;
		}
		else {
			loggerError << (LogMsg() << "Ошибка (FileWatcher::callback_thread): \"" << this->_filepath << "\" ; " << ec.message());
			break;
		}
	}
}

void FileWatcher::destroy() {
	this->_destroy.store(true);

	if (this->_callback_thread.joinable()) {
		this->_callback_thread.join();
	}
}

void FileWatcher::release() {
	this->destroy();
}