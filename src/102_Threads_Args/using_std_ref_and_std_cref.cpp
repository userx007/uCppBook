/*

g++ -pthread using_std_ref_and_std_cref.cpp -o app

*/

#include <iostream>
#include <thread>
#include <functional>
#include <vector>

void increment(int& value) {
    for (int i = 0; i < 1000; ++i) {
        ++value;
    }
}

void print_info(const std::string& name, const std::vector<int>& data) {
    std::cout << name << " has " << data.size() << " elements" << std::endl;
}

int main() {
    int counter = 0;
    
    // CORRECT: Use std::ref to pass by reference
    std::thread t1(increment, std::ref(counter));
    t1.join();
    
    std::cout << "Counter after thread: " << counter << std::endl;
    
    // Using std::cref for const references
    std::string name = "MyVector";
    std::vector<int> vec = {1, 2, 3, 4, 5};
    
    std::thread t2(print_info, std::cref(name), std::cref(vec));
    t2.join();
    
    return 0;
}