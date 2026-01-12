/*
g++ -pthread --std=c++20 detachable_background_tasks.cpp -o app
*/

#include <thread>
#include <iostream>
#include <chrono>

class DetachableThread {
private:
    std::thread thread_;
    
public:
    template<typename Callable, typename... Args>
    explicit DetachableThread(Callable&& func, Args&&... args)
        : thread_(std::forward<Callable>(func), std::forward<Args>(args)...) {
        thread_.detach();  // Immediately detach
    }
    
    // No destructor needed - thread is already detached
    ~DetachableThread() = default;
    
    DetachableThread(const DetachableThread&) = delete;
    DetachableThread& operator=(const DetachableThread&) = delete;
};

void background_logger(std::string message) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << "Background: " << message << "\n";
}

int main() {
    {
        DetachableThread t(background_logger, "Hello from detached thread");
        std::cout << "Main thread doesn't wait\n";
    } // Destructor doesn't block
    
    std::cout << "Continuing immediately...\n";
    
    // Give detached thread time to complete
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    return 0;
}