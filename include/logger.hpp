#pragma once

#include <atomic>
#include <fstream>
#include <string>
#include <thread>
#include "lf_queue.hpp"
#include "logger_common.hpp"
#include "thread_utils.hpp"

namespace ricox {

class logger final {
   private:
	const std::string file_name;
	std::ofstream file;
	lf_queue<log_entry> queue;
	std::thread worker_thread;
	std::atomic<bool> is_running;

	auto flush_queue() -> void {
		while (is_running.load(std::memory_order_acquire)) {
			while (!queue.empty()) {
				auto entry = queue.pop();
				if (!entry.has_value()) continue;
				const auto& log = entry.value();
				switch (log.type) {
					case log_type::CHAR:
						file << log.value.c;
						break;
					case log_type::INTEGER:
						file << log.value.i;
						break;
					case log_type::LONG_INTEGER:
						file << log.value.l;
						break;
					case log_type::LONG_LONG_INTEGER:
						file << log.value.ll;
						break;
					case log_type::UNSIGNED_INTEGER:
						file << log.value.ui;
						break;
					case log_type::UNSIGNED_LONG_INTEGER:
						file << log.value.ul;
						break;
					case log_type::UNSIGNED_LONG_LONG_INTEGER:
						file << log.value.ull;
						break;
					case log_type::FLOAT:
						file << log.value.f;
						break;
					case log_type::DOUBLE:
						file << log.value.d;
						break;
				}
			}

			std::this_thread::sleep_for(std::chrono::milliseconds{1});
		}
	}

   public:
	explicit logger(const std::string& file_name_, size_t queue_size = LOG_QUEUE_SIZE)
		: file_name{file_name_}, queue{queue_size}, is_running{false} {
		file.open(file_name, std::ios::out | std::ios::app);
		if (!file.is_open()) throw std::runtime_error("Failed to open log file: " + file_name);

		is_running.store(true, std::memory_order_release);
		auto thread = std::move(create_and_start_thread(-1, "logger", [this]() -> void { flush_queue(); }));
		if (!thread.has_value()) throw std::runtime_error("Failed to start logger thread");
		worker_thread = std::move(thread.value());
	}

	~logger() {
		while (!queue.empty()) std::this_thread::sleep_for(std::chrono::seconds{1});  // Wait for queue to be processed
		is_running.store(false, std::memory_order_release);
		if (worker_thread.joinable()) worker_thread.join();
		file.close();

		std::cerr << "Logger thread has been stopped and file closed.\n";
	}

	logger() = delete;
	logger(const logger&) = delete;
	logger(logger&&) = delete;
	logger& operator=(const logger&) = delete;
	logger& operator=(logger&&) = delete;

	auto push_value(const log_entry& entry) noexcept -> void {
		if (!queue.push(entry)) std::cerr << "Logger queue is full, dropping log entry.\n";
	}

	auto push_value(const char value) noexcept -> void { push_value(log_entry{log_type::CHAR, {.c = value}}); }

	auto push_value(const char* value) noexcept -> void {
		while (*value) {
			push_value(*value);
			++value;
		}
	}

	auto push_value(const std::string& value) noexcept -> void { push_value(value.c_str()); }

	auto push_value(const int value) noexcept -> void { push_value(log_entry{log_type::INTEGER, {.i = value}}); }

	auto push_value(const long value) noexcept -> void { push_value(log_entry{log_type::LONG_INTEGER, {.l = value}}); }

	auto push_value(const long long value) noexcept -> void {
		push_value(log_entry{log_type::LONG_LONG_INTEGER, {.ll = value}});
	}

	auto push_value(const unsigned int value) noexcept -> void {
		push_value(log_entry{log_type::UNSIGNED_INTEGER, {.ui = value}});
	}

	auto push_value(const unsigned long value) noexcept -> void {
		push_value(log_entry{log_type::UNSIGNED_LONG_INTEGER, {.ul = value}});
	}

	auto push_value(const unsigned long long value) noexcept -> void {
		push_value(log_entry{log_type::UNSIGNED_LONG_LONG_INTEGER, {.ull = value}});
	}

	auto push_value(const float value) noexcept -> void { push_value(log_entry{log_type::FLOAT, {.f = value}}); }

	auto push_value(const double value) noexcept -> void { push_value(log_entry{log_type::DOUBLE, {.d = value}}); }

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

	auto log(const char* s) noexcept -> void {
		while (*s) {
			if (*s == '%') {
				if (*(s + 1) == '%') [[unlikely]] {
					++s;
				} else {
					std::cerr << "Logger: Missing value for format specifier.\n";
					return;
				}
			}
			push_value(*s++);
		}
	}
};

}  // namespace ricox