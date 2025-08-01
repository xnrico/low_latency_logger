#include "lf_queue.hpp"
#include <thread>
#include <ranges>
#include <chrono>
#include <iostream>
#include <functional>

using ricox::lf_queue;

static constexpr size_t ITERS = 1000000;
static constexpr size_t MOD = 123456;

auto produce(lf_queue<size_t>& queue) -> void {
    for (auto i : std::views::iota(0U, ITERS)) {
        // std::cout << "Pushed " << i << "? " << queue.push(i) << "\n";
        queue.push(i);
    }

    queue.push(std::numeric_limits<size_t>::max());
    std::cout << "Producer Ending\n";
}

auto consume(lf_queue<size_t>& queue) -> void {
    auto item = 0;

    while (item < std::numeric_limits<size_t>::max()) {
        auto elem = queue.pop();
        if (!elem.has_value()) continue;
        item = elem.value();
        if (item % MOD == 0) {
            std::cout << "Got number: " << item << " || Queue size: " << queue.size() << "\n";
        }
        // std::this_thread::sleep_for(std::chrono::nanoseconds{5});
    }

    std::cout << "Consumer Ending\n";
}

auto main(int argc, char** argv) -> int {
    auto queue = lf_queue<size_t>{};

    auto producer_thread = std::thread(produce, std::ref(queue));
    auto consumer_thread = std::thread(consume, std::ref(queue));

    producer_thread.join();
    consumer_thread.join();

    return 0;
}

