#include <iostream>
#include <string>
using namespace std;

class Calculator {
public:
    // Overloaded add functions for different types
    int add(int a, int b) {
        cout << "Adding two integers" << endl;
        return a + b;
    }
    
    double add(double a, double b) {
        cout << "Adding two doubles" << endl;
        return a + b;
    }
    
    string add(const string& a, const string& b) {
        cout << "Concatenating two strings" << endl;
        return a + b;
    }
    
    // Overloaded with different number of parameters
    int add(int a, int b, int c) {
        cout << "Adding three integers" << endl;
        return a + b + c;
    }
};

int main() {
    Calculator calc;
    
    // Compiler determines which function to call at compile time
    cout << calc.add(5, 3) << endl;                    // Calls int version
    cout << calc.add(5.5, 3.2) << endl;                // Calls double version
    cout << calc.add("Hello, ", "World!") << endl;     // Calls string version
    cout << calc.add(1, 2, 3) << endl;                 // Calls 3-parameter version
    
    return 0;
}