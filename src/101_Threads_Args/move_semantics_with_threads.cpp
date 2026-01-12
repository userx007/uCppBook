/*

g++ -pthread move_semantics_with_threads.cpp -o app

*/

#include <iostream>
#include <thread>
#include <memory>
#include <vector>
#include <string>

class LargeData {
    std::vector<int> data;
public:
    LargeData(size_t size) : data(size, 42) {
        std::cout << "LargeData created with " << size << " elements" << std::endl;
    }
    
    // Move constructor
    LargeData(LargeData&& other) noexcept : data(std::move(other.data)) {
        std::cout << "LargeData moved" << std::endl;
    }
    
    size_t size() const { return data.size(); }
};

void process_data(LargeData data) {
    std::cout << "Processing data with " << data.size() << " elements" << std::endl;
}

void process_unique(std::unique_ptr<std::string> ptr) {
    std::cout << "Processing: " << *ptr << std::endl;
}

int main() {
    // Moving a large object
    LargeData large(1000000);
    std::thread t1(process_data, std::move(large));
    // 'large' is now in a moved-from state and should not be used
    t1.join();
    
    // Moving a unique_ptr (move-only type)
    auto ptr = std::make_unique<std::string>("Important data");
    std::thread t2(process_unique, std::move(ptr));
    // 'ptr' is now nullptr
    t2.join();
    
    return 0;
}