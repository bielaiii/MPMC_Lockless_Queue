
#include "LockLess.hpp"
#include <chrono>
#include <print>
#include <thread>
using namespace std::chrono_literals;

template <typename Element, size_t N>
void running_func(Tools::detail::Queue<Element, N> &lockless_q, Element i) {

    lockless_q.enqueue(i);
}

int main() {

    auto Queue_ = Tools::detail::Queue<int, 32>();

    {

        std::vector<std::jthread> pools(20);
        for (int i = 0; i < 4; i++) {
            pools[i] = std::jthread(running_func<int, 32>, std::ref(Queue_), i);
        }
    }

    std::println("real sz : {}", Queue_.size());
    std::print("===============\n");
    while (!Queue_.empty()) {
        int value;
        int i = Queue_.dequeue(value);
        std::println("ret : {} value {}", i, value);
    }

    return 0;
}
