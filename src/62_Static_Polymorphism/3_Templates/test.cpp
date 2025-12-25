#include <iostream>
#include <vector>
#include <string>
using namespace std;

// Template function - works with any type
template <typename T>
T maximum(T a, T b) {
    return (a > b) ? a : b;
}

// Template class
template <typename T>
class Container {
private:
    T value;
    
public:
    Container(T val) : value(val) {}
    
    T getValue() const { return value; }
    void setValue(T val) { value = val; }
    
    void print() const {
        cout << "Value: " << value << endl;
    }
};

// Template with multiple parameters
template <typename T, typename U>
class Pair {
private:
    T first;
    U second;
    
public:
    Pair(T f, U s) : first(f), second(s) {}
    
    void display() const {
        cout << "(" << first << ", " << second << ")" << endl;
    }
};

// Function template with template template parameter
template <typename T>
void printContainer(const vector<T>& vec) {
    cout << "Container contents: ";
    for (const auto& elem : vec) {
        cout << elem << " ";
    }
    cout << endl;
}

int main() {
    // Template functions - type resolved at compile time
    cout << "Max of 10 and 20: " << maximum(10, 20) << endl;
    cout << "Max of 3.5 and 2.1: " << maximum(3.5, 2.1) << endl;
    cout << "Max of 'a' and 'z': " << maximum('a', 'z') << endl;
    
    // Template classes
    Container<int> intContainer(42);
    Container<string> strContainer("Hello");
    Container<double> dblContainer(3.14);
    
    intContainer.print();
    strContainer.print();
    dblContainer.print();
    
    // Template with multiple types
    Pair<int, string> p1(1, "First");
    Pair<double, char> p2(3.14, 'A');
    
    p1.display();
    p2.display();
    
    // Vector with different types
    vector<int> intVec = {1, 2, 3, 4, 5};
    vector<string> strVec = {"apple", "banana", "cherry"};
    
    printContainer(intVec);
    printContainer(strVec);
    
    return 0;
}