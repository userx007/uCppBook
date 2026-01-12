/*

g++ -pthread different_callable_types.cpp -o app

*/

#include <iostream>
#include <thread>

// ----------------------------------------
// 1. Function pointer
void functionThread() {
    std::cout << "Function thread" << std::endl;
}

void functionparamThread(int x) {
    std::cout << "Function param thread [" << x << "]"<< std::endl;
}

// ----------------------------------------
// 2. Functor (function object)
class FunctorThread {
public:
    void operator()() const {
        std::cout << "Functor thread" << std::endl;
    }
};

// 2a. Functor (function object)
class FunctorThreadParam {
public:
    void operator()(int x) const {
        std::cout << "Functor param thread:" << x << std::endl;
    }
};

// ----------------------------------------
// 3. Member function
class MyClass {
public:
    void memberFunction(int value) {
        std::cout << "Member function thread: " << value << std::endl;
    }
};


// ----------------------------------------
int main() {

    // ----------------------------------------

    // Using a function
    std::thread t1(functionThread);

    // ----------------------------------------

    // Using a function with parameter
    std::thread t1p(functionparamThread, 5);

    // ----------------------------------------

    // Using a lambda
    std::thread t2([]() {
        std::cout << "Lambda thread" << std::endl;
    });
    
    // ----------------------------------------

    // Using a lambda capturing param
    int x = 10;
    std::thread t2p([x]() {
        std::cout << "Lambda thread with param:" << x << std::endl;
    });

    // ----------------------------------------

    // Using a functor
    FunctorThread functor;
    std::thread t3(functor);
    
    // ----------------------------------------

    // Using a functor with parameter
    FunctorThreadParam functor_param;
    std::thread t3p(functor_param, 32);    

    // ----------------------------------------

    // Using a member function
    MyClass obj;
    std::thread t4(&MyClass::memberFunction, &obj, 42);
    
    // ----------------------------------------
    
    // Join all threads
    t1.join();
    t1p.join();
    t2.join();
    t2p.join();
    t3.join();
    t3p.join();
    t4.join();
    
    return 0;
}