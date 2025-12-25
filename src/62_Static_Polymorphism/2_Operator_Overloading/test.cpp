#include <iostream>
using namespace std;

class Complex {
private:
    double real, imag;
    
public:
    Complex(double r = 0, double i = 0) : real(r), imag(i) {}
    
    // Overload + operator
    Complex operator+(const Complex& other) const {
        return Complex(real + other.real, imag + other.imag);
    }
    
    // Overload - operator
    Complex operator-(const Complex& other) const {
        return Complex(real - other.real, imag - other.imag);
    }
    
    // Overload * operator
    Complex operator*(const Complex& other) const {
        return Complex(
            real * other.real - imag * other.imag,
            real * other.imag + imag * other.real
        );
    }
    
    // Overload == operator
    bool operator==(const Complex& other) const {
        return (real == other.real && imag == other.imag);
    }
    
    // Overload << operator for output
    friend ostream& operator<<(ostream& os, const Complex& c) {
        os << c.real;
        if (c.imag >= 0) os << " + " << c.imag << "i";
        else os << " - " << -c.imag << "i";
        return os;
    }
};

int main() {
    Complex c1(3, 4);
    Complex c2(1, 2);
    
    // Using overloaded operators - resolved at compile time
    Complex c3 = c1 + c2;
    Complex c4 = c1 - c2;
    Complex c5 = c1 * c2;
    
    cout << "c1 = " << c1 << endl;
    cout << "c2 = " << c2 << endl;
    cout << "c1 + c2 = " << c3 << endl;
    cout << "c1 - c2 = " << c4 << endl;
    cout << "c1 * c2 = " << c5 << endl;
    cout << "c1 == c2: " << (c1 == c2) << endl;
    
    return 0;
}