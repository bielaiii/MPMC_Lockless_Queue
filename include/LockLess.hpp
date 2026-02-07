#ifndef LOCKLESS_QUEUE
#define LOCKLESS_QUEUE

#include <array>
#include <atomic>
#include <functional>
#include <new>
namespace Tools {
namespace detail {

#if defined(__APPLE__) && defined(__MACH__)
// hardware_destructive_interference_size is 64 in common
// on apple silion, actual cacheline size is 128
#define INTEREFERENCE 128
#else
#define INTEREFERENCE std::hardware_destructive_interference_size
#endif

template<typename Element>
struct QueueElement {
    
};

template <typename Element, size_t N>
class Queue {
    char pad0[INTEREFERENCE];
    size_t size_{N};
    alignas(INTEREFERENCE) std::atomic<int> producer_ptr{};
    alignas(INTEREFERENCE) std::atomic<int> inner_ptr;
    alignas(INTEREFERENCE) std::atomic<int> consumer_ptr{};
    char pad1[INTEREFERENCE - sizeof(std::atomic<int>)];
    std::array<Element, N> inner_queue;
public:

    void publish_enqueue(int idx) noexcept {
        int old_value = idx;
        int ticket = idx;
        while(!producer_ptr.compare_exchange_strong(old_value, ticket,  std::memory_order_relaxed)) {
            if (old_value == ticket + 1) {
                
                break;;
            }
        }
        producer_ptr.fetch_add(1);
    }


    template<typename... Args>
    bool enqueue(Args&&... args)noexcept {
        auto ticket = inner_ptr.load(std::memory_order::acquire);
        auto old_value = ticket;
        ticket++;
        new (static_cast<char*>(&inner_queue[ticket])) Element(std::forward<Args>(args)...);
        inner_ptr.fetch_add(1,std::memory_order::acq_rel);
        publish_enqueue(ticket);
        return 1;
    }  
    
    template<typename pushed_element>
    bool enqueue(pushed_element&& element) noexcept {
        
        auto ticket = inner_ptr.fetch_add(1,std::memory_order::acquire);
        auto old_value = ticket;
        
        ticket++;
        inner_queue[ticket] = std::forward<pushed_element>(element);
        
        publish_enqueue(ticket);
        return 1;
        
    }

    int head_idx() noexcept {
        return consumer_ptr.load(std::memory_order::acquire);
    }

    bool dequeue(Element &element) {
        int check_last = consumer_ptr.fetch_add(1, std::memory_order_release);
        if (check_last == size_ - 1) {
            consumer_ptr.store(0, std::memory_order_acquire);
        }
        
        element = std::move(inner_queue[check_last]);
        return 1;
    }
    bool try_enqueue(Element element) {}
    bool try_dequeue(Element & element) {}

    bool unsafe_enqueue(Element element) {}
    bool unsafe_dequeue(Element& element) {}


    bool empty() noexcept {
        return consumer_ptr.load() == producer_ptr.load();
    }
    size_t size() noexcept { return size_; }
};
} // namespace detail
} // namespace Tools

#endif