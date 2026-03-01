
#include "LockLess.hpp"
#include <chrono>
#include <print>
#include <thread>
using namespace std::chrono_literals;

struct PrintDst {
    int i ;
    PrintDst() noexcept = default;
    PrintDst(int i_ ) noexcept : i(i_ * 100) {}
    ~PrintDst() noexcept { std::println("print in terminal {}", i); }
};

template <typename Element, size_t N>
void running_func(Tools::detail::Queue<Element, N> &lockless_q, int i) {

    lockless_q.enqueue(Element(i));
}

int main() {

    auto Queue_ = Tools::detail::Queue<PrintDst, 32>();

    {

        std::vector<std::jthread> pools(20);
        for (int i = 0; i < 4; i++) {
            pools[i] = std::jthread(running_func<PrintDst, 32>, std::ref(Queue_), i);
        }
    }

    std::println("real sz : {}", Queue_.size());
    std::print("===============\n");
    while (!Queue_.empty()) {
        PrintDst value;
        int i = Queue_.dequeue(value);
        std::println("ret : {} value {}", i, value.i);
    }

    return 0;
}
