// Template implementation for lf_queue

template <typename T>
constexpr lf_queue<T>::lf_queue(size_t size) : r_pos{0}, w_pos{0}, capacity_{1U} {
	// Ensure the size is a power of two for efficient wrapping
	// idx = tail & (N - 1)
	while (capacity_ < size) capacity_ <<= 1;
	data.resize(capacity_);
}

template <typename T>
constexpr lf_queue<T>::lf_queue() : lf_queue(DEFAULT_SIZE) {}

template <typename T>
auto lf_queue<T>::size() const noexcept -> size_t {
	return w_pos.load(std::memory_order_relaxed) - r_pos.load(std::memory_order_relaxed);
}

template <typename T>
auto lf_queue<T>::empty() const noexcept -> bool {
	return size() == 0;
}

template <typename T>
auto lf_queue<T>::full() const noexcept -> bool {
	return size() == capacity_;
}

template <typename T>
auto lf_queue<T>::push(T elem) noexcept -> bool {
	auto tail = this->w_pos.load(std::memory_order_relaxed);
	if (tail - r_pos.load(std::memory_order_relaxed) >= capacity_) [[unlikely]]
		return false;  // Queue is full
	data[tail & (capacity_ - 1)] = std::move(elem);
	this->w_pos.store(tail + 1, std::memory_order_release);
	return true;
}

template <typename T>
auto lf_queue<T>::pop() noexcept -> std::optional<T> {
	auto head = this->r_pos.load(std::memory_order_relaxed);
	if (head == this->w_pos.load(std::memory_order_acquire)) return std::nullopt;  // Queue is empty
	auto& elem = data[head & (capacity_ - 1)];
	this->r_pos.store(head + 1, std::memory_order_release);
	return std::optional<T>{std::move(elem)};
}
