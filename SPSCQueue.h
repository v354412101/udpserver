#ifndef SPSCQUEUE_H
#define SPSCQUEUE_H

#include <vector>
#include <atomic>

template<typename T>
class SPSCQueue {
public:
    explicit SPSCQueue(int size)
        : buffer_(size), size_(size), head_(0), tail_(0) {}

    bool push(const T& item) {
        int head = head_.load(std::memory_order_relaxed);
        int next_head = (head + 1) % size_;

        if (next_head == tail_.load(std::memory_order_acquire)) {
            return false;
        }

        buffer_[head] = item;
        head_.store(next_head, std::memory_order_release);
        return true;
    }

    bool pop(T& item) {
        int tail = tail_.load(std::memory_order_relaxed);

        if (tail == head_.load(std::memory_order_acquire)) {
            return false;
        }

        item = buffer_[tail];
        tail_.store((tail + 1) % size_, std::memory_order_release);
        return true;
    }

private:
    std::vector<T> buffer_;
    const int size_;
    std::atomic<int> head_;
    std::atomic<int> tail_;
};


#endif