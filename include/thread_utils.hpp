#pragma once

#include <sys/syscall.h>
#include <unistd.h>
#include <thread>
#include <iostream>
#include <optional>

namespace ricox {

template <typename T, typename... Args>
inline auto create_and_start_thread(int core_id, const std::string& name, T&& func, Args&&... args) -> std::optional<std::thread> {
	auto running = std::atomic<bool>{false};
    auto failed = std::atomic<bool>{false};

    auto set_affinity = [] (int core_id) -> bool {
        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);
        CPU_SET(core_id, &cpuset);

        return pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset) == 0;
    };

    auto thread_func = [&] () -> void {
        if (core_id >= 0 && !set_affinity(core_id)) {
            failed.store(true, std::memory_order_release);
            std::cerr << "Failed to set thread affinity for core " << core_id << "\n";
            return;
        }

        std::cout << "Thread " << name << " starting on core " << core_id << "\n";
        running.store(true, std::memory_order_release);
        std::forward<T>(func)(std::forward<Args>(args)...);
    };

    auto thread = std::thread(thread_func);
    while (!running.load(std::memory_order_acquire) && !failed.load(std::memory_order_acquire)) {
        std::this_thread::sleep_for(std::chrono::seconds{1}); // Wait for thread to start
    }

    if (failed.load(std::memory_order_acquire)) {
        thread.join();
        return std::nullopt;
    }
    
    return std::optional<std::thread>{std::move(thread)};
}

}  // namespace ricox