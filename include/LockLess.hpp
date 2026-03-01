#ifndef LOCKLESS_QUEUE
#define LOCKLESS_QUEUE

#include <array>
#include <atomic>
#include <functional>
#include <new>
#include <print>
#include <type_traits>
namespace Tools {
namespace detail {

#if defined(__APPLE__) && defined(__MACH__)
// hardware_destructive_interference_size is 64 in common
// on apple silion, actual cacheline size is 128
#define INTEREFERENCE 128
#else
#define INTEREFERENCE std::hardware_destructive_interference_size
#endif

template <typename Element>
struct QueueElement {};

template <typename Element, size_t N>
class Queue {
    char pad0[INTEREFERENCE];
    size_t size_{N};
    alignas(INTEREFERENCE) std::atomic<int> producer_ptr{};
    alignas(INTEREFERENCE) std::atomic<int> inner_ptr{};
    alignas(INTEREFERENCE) std::atomic<int> consumer_ptr{};
    char pad1[INTEREFERENCE - sizeof(std::atomic<int>)];
    std::array<Element, N> inner_queue;
    // std::vector<Element> inner_queue;

public:
    Queue() noexcept = default;

    void publish_enqueue(int idx) noexcept {
        int expected = idx;
        int desired  = idx + 1;

        // here is linearization point,
        while (!producer_ptr.compare_exchange_strong(
            expected, desired, std::memory_order_release)) {}
    }

    // blocking until success
    template <typename... Args>
    bool enqueue(Args &&...args) noexcept {
        auto ticket = inner_ptr.fetch_add(1, std::memory_order_acquire);

        new (static_cast<void *>(&inner_queue[ticket]))
            Element(std::forward<Args>(args)...);

        publish_enqueue(ticket);
        return true;
    }
    // blocking until success
    template <typename pushed_element>
    bool enqueue(pushed_element &&element) noexcept {
        auto ticket = inner_ptr.fetch_add(1, std::memory_order_acquire);

        inner_queue[ticket] = std::forward<pushed_element>(element);

        publish_enqueue(ticket);

        return true;
    }

    int head_idx() noexcept {
        return consumer_ptr.load(std::memory_order_release);
    }

    bool dequeue(Element &element) noexcept {

        if (empty()) {
            return false;
        }

        int last_one = consumer_ptr.fetch_add(1, std::memory_order_acquire);
        if (last_one == size_ - 1) {
            consumer_ptr.store(0, std::memory_order_acquire);
        }

        element = std::move(inner_queue[last_one]);
        return true;
    }

    bool publish_once(int idx) noexcept {
        int expected = idx;
        int desired  = idx + 1;
        auto result  = producer_ptr.compare_exchange_strong(
            expected, desired, std::memory_order_release);
        if (result) {
            inner_ptr.fetch_add(1, std::memory_order_release);
            if constexpr (!std::is_trivially_destructible_v<Element>) {
                ~inner_queue[expected];
            }
        }
        return result;
    }

    // non-blocking
    // only try once
    bool try_enqueue(Element &element) {
        auto ticket = inner_ptr.load(std::memory_order_acquire);

        inner_ptr.fetch_add(1);

        inner_queue[ticket] = std::move(element);

        int res_ = publish_once(ticket);

        if (res_) {
            return 1;
        }
    }
    bool try_dequeue(Element &element) noexcept {}

    // assuming operation will be success anyway
    bool unsafe_enqueue(Element element) noexcept {}
    bool unsafe_dequeue(Element &element) noexcept {}

    size_t size() noexcept {
        return producer_ptr.load(std::memory_order_acquire) -
               consumer_ptr.load(std::memory_order_acquire);
    }
    bool empty() noexcept {
        return consumer_ptr.load(std::memory_order_acquire) ==
               producer_ptr.load(std::memory_order_acquire);
    }
    ~Queue() noexcept {
        if constexpr (!std::is_trivially_destructible_v<Element>) {
            for (auto &v : inner_queue) {
                v.~Element();
            }
        }
    }
};
} // namespace detail
} // namespace Tools

#endif