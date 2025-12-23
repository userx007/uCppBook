// ==========================================
// WITHOUT Pimpl - Traditional Approach
// ==========================================

// Widget.h (traditional)
#include <string>
#include <vector>
#include <memory>
#include "ComplexDependency.h"  // Exposes implementation details

class Widget {
public:
    Widget();
    void doSomething();
    
private:
    std::string name_;
    std::vector<int> data_;
    ComplexDependency dep_;  // All users must include this header
    int privateValue_;
};

// ==========================================
// WITH Pimpl - Modern Approach
// ==========================================

// Widget.h (with Pimpl)
#include <memory>  // Only need this!

class Widget {
public:
    Widget();
    ~Widget();  // Must declare destructor
    
    // Rule of Five (for proper resource management)
    Widget(const Widget& other);
    Widget& operator=(const Widget& other);
    Widget(Widget&& other) noexcept;
    Widget& operator=(Widget&& other) noexcept;
    
    void doSomething();
    void setName(const std::string& name);
    std::string getName() const;
    
private:
    class Impl;  // Forward declaration
    std::unique_ptr<Impl> pImpl_;  // Pointer to implementation
};

// ==========================================
// Widget.cpp (implementation file)
// ==========================================

#include "Widget.h"
#include <string>
#include <vector>
#include "ComplexDependency.h"  // Only included in .cpp file

// Define the implementation class
class Widget::Impl {
public:
    void doSomething() {
        data_.push_back(privateValue_++);
        dep_.process(name_);
    }
    
    std::string name_;
    std::vector<int> data_;
    ComplexDependency dep_;
    int privateValue_ = 0;
};

// Constructor
Widget::Widget() 
    : pImpl_(std::make_unique<Impl>()) {
}

// Destructor (must be in .cpp where Impl is complete)
Widget::~Widget() = default;

// Copy constructor
Widget::Widget(const Widget& other)
    : pImpl_(std::make_unique<Impl>(*other.pImpl_)) {
}

// Copy assignment
Widget& Widget::operator=(const Widget& other) {
    if (this != &other) {
        *pImpl_ = *other.pImpl_;
    }
    return *this;
}

// Move constructor
Widget::Widget(Widget&& other) noexcept = default;

// Move assignment
Widget& Widget::operator=(Widget&& other) noexcept = default;

// Public interface implementations
void Widget::doSomething() {
    pImpl_->doSomething();
}

void Widget::setName(const std::string& name) {
    pImpl_->name_ = name;
}

std::string Widget::getName() const {
    return pImpl_->name_;
}

// ==========================================
// Usage Example
// ==========================================

#include <iostream>

int main() {
    Widget w1;
    w1.setName("First Widget");
    w1.doSomething();
    
    // Copy semantics work correctly
    Widget w2 = w1;
    w2.setName("Second Widget");
    
    std::cout << "w1 name: " << w1.getName() << "\n";
    std::cout << "w2 name: " << w2.getName() << "\n";
    
    // Move semantics
    Widget w3 = std::move(w2);
    std::cout << "w3 name: " << w3.getName() << "\n";
    
    return 0;
}

// ==========================================
// Alternative: Using shared_ptr for shared ownership
// ==========================================

class SharedWidget {
public:
    SharedWidget() : pImpl_(std::make_shared<Impl>()) {}
    
    // Compiler-generated copy operations work naturally
    // Multiple objects can share the same implementation
    
private:
    class Impl;
    std::shared_ptr<Impl> pImpl_;
};