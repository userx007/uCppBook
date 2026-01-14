#include <iostream>
#include <memory>

class A {
public:
    A() { std::cout << "A created!" << std::endl; }
    ~A() { std::cout << "A destroyed!" << std::endl; }
};

int main() {
    std::shared_ptr<A> sp1 = std::make_shared<A>();  // shared ownership
    std::weak_ptr<A> wp1 = sp1;  // weak pointer, does not affect reference count

    // Check if the object is still alive
    if (auto sp2 = wp1.lock()) {
        std::cout << "Object is still alive!" << std::endl;
    } else {
        std::cout << "Object has been destroyed." << std::endl;
    }

    // Reset shared pointer, which will destroy the object
    sp1.reset();

    if (auto sp2 = wp1.lock()) {
        std::cout << "Object is still alive!" << std::endl;
    } else {
        std::cout << "Object has been destroyed." << std::endl;
    }

    return 0;
}