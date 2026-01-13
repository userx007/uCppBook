/*
g++ -pthread --std=c++20 thread_safe_queue.cpp -o app
*/

#include <iostream>
#include <mutex>
#include <queue>
#include <thread>
#include <chrono>
#include <optional>

template<typename T>
class ThreadSafeQueue {
private:
    std::queue<T> queue;
    mutable std::mutex mtx;

public:
    void push(T value) {
        std::lock_guard<std::mutex> lock(mtx);
        queue.push(std::move(value));
    }
    
    std::optional<T> pop() {
        std::lock_guard<std::mutex> lock(mtx);
        if (queue.empty()) {
            return std::nullopt;
        }
        T value = std::move(queue.front());
        queue.pop();
        return value;
    }
    
    bool empty() const {
        std::lock_guard<std::mutex> lock(mtx);
        return queue.empty();
    }
    
    size_t size() const {
        std::lock_guard<std::mutex> lock(mtx);
        return queue.size();
    }
};

int main() {
    ThreadSafeQueue<int> queue;
    
    // Producer thread
    std::thread producer([&queue]() {
        for (int i = 0; i < 10; ++i) {
            queue.push(i);
            std::cout << "Produced: " << i << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    });
    
    // Consumer thread
    std::thread consumer([&queue]() {
        for (int i = 0; i < 10; ++i) {
            while (true) {
                auto value = queue.pop();
                if (value) {
                    std::cout << "Consumed: " << *value << std::endl;
                    break;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        }
    });
    
    producer.join();
    consumer.join();
    
    return 0;
}