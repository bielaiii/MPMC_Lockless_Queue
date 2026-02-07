
#include "LockLess.hpp"
#include <chrono>
#include <print>
#include <thread>
using namespace std::chrono_literals;

template <typename Element, size_t N>
void running_func(Tools::detail::Queue<Element, N> &lockless_q, Element i) {
    // std::this_thread::sleep_for(5s);
    //std::print("value {}", i);
    lockless_q.enqueue(i);
}

int main() {

    std::vector<std::jthread> pools(20);

    auto Queue_ = Tools::detail::Queue<int, 32>();

    for (int i = 0; i < 4; i++) {
        pools[i] = std::jthread(running_func<int, 32>, std::ref(Queue_), i);
    }

    while (!Queue_.empty()) {
        int value;
        int i = Queue_.dequeue(value);
        std::println("value {}", value);
    }

    return 0;
}
