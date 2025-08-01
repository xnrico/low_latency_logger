#pragma once

#include <atomic>
#include <optional>
#include <vector>

namespace ricox {

template <typename T>
class lf_queue final {
   private:
	static constexpr size_t DEFAULT_SIZE = 1024 * 1024;	 // 1 MB default size

	std::vector<T> data;
	std::atomic<size_t> r_pos;	// head of the queue
	std::atomic<size_t> w_pos;	// tail of the queue
	size_t capacity_;

   public:
	explicit constexpr lf_queue(size_t size);
	explicit constexpr lf_queue();
	~lf_queue() = default;

	lf_queue(const lf_queue&) = delete;
	lf_queue(const lf_queue&&) = delete;
	lf_queue& operator=(const lf_queue&) = delete;
	lf_queue& operator=(const lf_queue&&) = delete;

	[[nodiscard]] constexpr auto capacity() const noexcept -> size_t { return capacity_; }
	[[nodiscard]] auto size() const noexcept -> size_t;
	[[nodiscard]] auto empty() const noexcept -> bool;
	[[nodiscard]] auto full() const noexcept -> bool;

	auto push(T elem) noexcept -> bool;
	auto pop() noexcept -> std::optional<T>;
};

#include "lf_queue.tpp"

}  // namespace ricox