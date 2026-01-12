/*

g++ -pthread different_callable_types.cpp -o app

*/

#include <iostream>
#include <thread>

// 1. Function pointer
void functionThread() {
    std::cout << "Function thread" << std::endl;
}

// 2. Functor (function object)
class FunctorThread {
public:
    void operator()() const {
        std::cout << "Functor thread" << std::endl;
    }
};

// 3. Member function
class MyClass {
public:
    void memberFunction(int value) {
        std::cout << "Member function thread: " << value << std::endl;
    }
};

int main() {
    // Using a function
    std::thread t1(functionThread);
    
    // Using a lambda
    std::thread t2([]() {
        std::cout << "Lambda thread" << std::endl;
    });
    
    // Using a functor
    FunctorThread functor;
    std::thread t3(functor);
    
    // Using a member function
    MyClass obj;
    std::thread t4(&MyClass::memberFunction, &obj, 42);
    
    // Join all threads
    t1.join();
    t2.join();
    t3.join();
    t4.join();
    
    return 0;
}