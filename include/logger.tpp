// Template implementation for the Logger class
template <typename Derived>
logger<Derived>::logger(size_t queue_size) : queue{queue_size}, is_running{false} {
	is_running.store(true, std::memory_order_release);
	auto thread = std::move(create_and_start_thread(-1, "logger", [this]() -> void { flush(); }));
	if (!thread.has_value()) throw std::runtime_error("Failed to start logger thread");
	worker_thread = std::move(thread.value());
}

template <typename Derived>
logger<Derived>::logger() : logger(LOG_QUEUE_SIZE) {}

template <typename Derived>
logger<Derived>::~logger() {}

template <typename Derived>
auto logger<Derived>::flush() -> void { 
	reinterpret_cast<Derived*>(this)->flush_queue(); 
}

template <typename Derived>
auto logger<Derived>::push_value(const log_entry& entry) noexcept -> void {
	if (!queue.push(entry)) std::cerr << "Logger queue is full, dropping log entry.\n";
}

template <typename Derived>
auto logger<Derived>::push_value(const char value) noexcept -> void { 
	push_value(log_entry{log_type::CHAR, {.c = value}}); 
}

template <typename Derived>
auto logger<Derived>::push_value(const char* value) noexcept -> void {
	while (*value) {
		push_value(*value);
		++value;
	}
}

template <typename Derived>
auto logger<Derived>::push_value(const std::string& value) noexcept -> void { 
	push_value(value.c_str()); 
}

template <typename Derived>
auto logger<Derived>::push_value(const int value) noexcept -> void { 
	push_value(log_entry{log_type::INTEGER, {.i = value}}); 
}

template <typename Derived>
auto logger<Derived>::push_value(const long value) noexcept -> void { 
	push_value(log_entry{log_type::LONG_INTEGER, {.l = value}}); 
}

template <typename Derived>
auto logger<Derived>::push_value(const long long value) noexcept -> void {
	push_value(log_entry{log_type::LONG_LONG_INTEGER, {.ll = value}});
}

template <typename Derived>
auto logger<Derived>::push_value(const unsigned int value) noexcept -> void {
	push_value(log_entry{log_type::UNSIGNED_INTEGER, {.ui = value}});
}

template <typename Derived>
auto logger<Derived>::push_value(const unsigned long value) noexcept -> void {
	push_value(log_entry{log_type::UNSIGNED_LONG_INTEGER, {.ul = value}});
}

template <typename Derived>
auto logger<Derived>::push_value(const unsigned long long value) noexcept -> void {
	push_value(log_entry{log_type::UNSIGNED_LONG_LONG_INTEGER, {.ull = value}});
}

template <typename Derived>
auto logger<Derived>::push_value(const float value) noexcept -> void { 
	push_value(log_entry{log_type::FLOAT, {.f = value}}); 
}

template <typename Derived>
auto logger<Derived>::push_value(const double value) noexcept -> void { 
	push_value(log_entry{log_type::DOUBLE, {.d = value}}); 
}

template <typename Derived>
auto logger<Derived>::destroy() -> void {
	// Give some time for remaining entries to be processed
	auto timeout = std::chrono::steady_clock::now() + std::chrono::seconds{2};
	while (!queue.empty() && std::chrono::steady_clock::now() < timeout) {
		std::this_thread::sleep_for(std::chrono::milliseconds{10});
	}

	is_running.store(false, std::memory_order_release);
	if (worker_thread.joinable()) worker_thread.join();
	std::cerr << "Logger thread has been stopped\n";
}

template <typename Derived>
auto logger<Derived>::log(const char* s) noexcept -> void {
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

// stdout_logger implementations
inline stdout_logger::stdout_logger() : logger<stdout_logger>{} {}

inline stdout_logger::~stdout_logger() { 
	destroy(); 
}

inline auto stdout_logger::flush_queue() -> void {
	while (is_running.load(std::memory_order_acquire)) {
		while (!queue.empty()) {
			auto entry = queue.pop();
			if (!entry.has_value()) continue;
			const auto& log = entry.value();
			switch (log.type) {
				case log_type::CHAR:
					std::cout << log.value.c;
					break;
				case log_type::INTEGER:
					std::cout << log.value.i;
					break;
				case log_type::LONG_INTEGER:
					std::cout << log.value.l;
					break;
				case log_type::LONG_LONG_INTEGER:
					std::cout << log.value.ll;
					break;
				case log_type::UNSIGNED_INTEGER:
					std::cout << log.value.ui;
					break;
				case log_type::UNSIGNED_LONG_INTEGER:
					std::cout << log.value.ul;
					break;
				case log_type::UNSIGNED_LONG_LONG_INTEGER:
					std::cout << log.value.ull;
					break;
				case log_type::FLOAT:
					std::cout << log.value.f;
					break;
				case log_type::DOUBLE:
					std::cout << log.value.d;
					break;
			}
		}

		std::this_thread::sleep_for(std::chrono::milliseconds{1});
	}
}

// file_logger implementations
inline file_logger::file_logger(const std::string& file_name) : logger<file_logger>{}, file_name{file_name} {
    auto path = std::filesystem::path(file_name);
	std::filesystem::create_directories(path.parent_path());	 // ensure the directory exists
	file.open(file_name, std::ios::out | std::ios::app);
	if (!file.is_open()) throw std::runtime_error("Failed to open log file: " + file_name);
}

inline file_logger::~file_logger() {
	destroy();
	if (file.is_open()) file.close();
}

inline auto file_logger::flush_queue() -> void {
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

