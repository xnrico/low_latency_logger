#pragma once

#include <atomic>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>
#include <typeinfo>
#include "lf_queue.hpp"
#include "logger_common.hpp"
#include "thread_utils.hpp"

namespace ricox {

template <typename Derived>
class logger {
   private:
	auto flush() -> void;

	auto push_value(const log_entry& entry) noexcept -> void;
	auto push_value(const char value) noexcept -> void;
	auto push_value(const char* value) noexcept -> void;
	auto push_value(const std::string& value) noexcept -> void;
	auto push_value(const int value) noexcept -> void;
	auto push_value(const long value) noexcept -> void;
	auto push_value(const long long value) noexcept -> void;
	auto push_value(const unsigned int value) noexcept -> void;
	auto push_value(const unsigned long value) noexcept -> void;
	auto push_value(const unsigned long long value) noexcept -> void;
	auto push_value(const float value) noexcept -> void;
	auto push_value(const double value) noexcept -> void;

   protected:
	lf_queue<log_entry> queue;
	std::thread worker_thread;
	std::atomic<bool> is_running;
	
	auto flush_queue() -> void;
	auto destroy() -> void;

   public:
	explicit logger(size_t queue_size);
	logger();
	~logger();

	logger(const logger&) = delete;
	logger(logger&&) = delete;
	logger& operator=(const logger&) = delete;
	logger& operator=(logger&&) = delete;

	template <typename T, typename... Args>
	auto log(const char* s, const T& value, Args... args) noexcept -> void {
		while (*s) {
			if (*s == '%') {
				if (*(s + 1) == '%') [[unlikely]] {
					++s;
				} else {
					push_value(value);
					log(s + 1, args...);
					return;
				}
			}

			push_value(*s++);
		}

		// Extra elements
	}

	auto log(const char* s) noexcept -> void;
};

class stdout_logger final : public logger<stdout_logger> {
   private:
	auto flush_queue() -> void;

   public:
	explicit stdout_logger();
	~stdout_logger();

	stdout_logger(const stdout_logger&) = delete;
	stdout_logger(stdout_logger&&) = delete;
	stdout_logger& operator=(const stdout_logger&) = delete;
	stdout_logger& operator=(stdout_logger&&) = delete;

	friend class logger<stdout_logger>;	 // Allow logger to access private members
};

class file_logger final : public logger<file_logger> {
   private:
	const std::string file_name;
	std::ofstream file;

	auto flush_queue() -> void;

   public:
	explicit file_logger(const std::string& file_name);
	~file_logger();

	file_logger() = delete;
	file_logger(const file_logger&) = delete;
	file_logger(file_logger&&) = delete;
	file_logger& operator=(const file_logger&) = delete;
	file_logger& operator=(file_logger&&) = delete;

	friend class logger<file_logger>;  // Allow logger to access private members
};

#include "logger.tpp"

}  // namespace ricox