#include <iostream>
using namespace std;

// Base template class using CRTP
template <typename Derived>
class Shape {
public:
    // Interface method that calls derived class
    void draw() const {
        static_cast<const Derived*>(this)->drawImpl();
    }
    
    double area() const {
        return static_cast<const Derived*>(this)->areaImpl();
    }
    
    void describe() const {
        cout << "This is a shape with area: " << area() << endl;
        draw();
    }
};

// Derived classes
class Circle : public Shape<Circle> {
private:
    double radius;
    
public:
    Circle(double r) : radius(r) {}
    
    void drawImpl() const {
        cout << "Drawing a circle with radius " << radius << endl;
    }
    
    double areaImpl() const {
        return 3.14159 * radius * radius;
    }
};

class Rectangle : public Shape<Rectangle> {
private:
    double width, height;
    
public:
    Rectangle(double w, double h) : width(w), height(h) {}
    
    void drawImpl() const {
        cout << "Drawing a rectangle " << width << "x" << height << endl;
    }
    
    double areaImpl() const {
        return width * height;
    }
};

class Triangle : public Shape<Triangle> {
private:
    double base, height;
    
public:
    Triangle(double b, double h) : base(b), height(h) {}
    
    void drawImpl() const {
        cout << "Drawing a triangle with base " << base 
             << " and height " << height << endl;
    }
    
    double areaImpl() const {
        return 0.5 * base * height;
    }
};

// Template function that works with any Shape
template <typename T>
void processShape(const Shape<T>& shape) {
    shape.describe();
    cout << "---" << endl;
}

int main() {
    Circle circle(5.0);
    Rectangle rectangle(4.0, 6.0);
    Triangle triangle(3.0, 8.0);
    
    // All resolved at compile time - no virtual function overhead!
    cout << "Circle:" << endl;
    circle.describe();
    cout << endl;
    
    cout << "Rectangle:" << endl;
    rectangle.describe();
    cout << endl;
    
    cout << "Triangle:" << endl;
    triangle.describe();
    cout << endl;
    
    // Using template function
    cout << "Processing shapes:" << endl;
    processShape(circle);
    processShape(rectangle);
    processShape(triangle);
    
    return 0;
}