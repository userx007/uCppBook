# Multiple Inheritance, Virtual Inheritance & Diamond Problem in C++

## Multiple Inheritance

Multiple inheritance in C++ allows a class to inherit from more than one base class. This means a derived class can access members and methods from multiple parent classes, combining their functionality.

**Basic Example:**

```cpp
#include <iostream>
#include <string>

class Engine {
public:
    void start() {
        std::cout << "Engine started\n";
    }
    int horsepower = 200;
};

class GPS {
public:
    void navigate(std::string destination) {
        std::cout << "Navigating to " << destination << "\n";
    }
};

// Car inherits from both Engine and GPS
class Car : public Engine, public GPS {
public:
    std::string model;
    Car(std::string m) : model(m) {}
};

int main() {
    Car myCar("Tesla Model S");
    myCar.start();           // From Engine
    myCar.navigate("Home");  // From GPS
    std::cout << "Horsepower: " << myCar.horsepower << "\n";
    return 0;
}
```

In this example, the Car class inherits from both Engine and GPS, gaining access to methods and data from both classes.

## The Diamond Problem

The diamond problem occurs when a class inherits from two classes that both inherit from a common base class, creating a diamond-shaped inheritance hierarchy. This leads to ambiguity because the most derived class ends up with two copies of the base class's members.

**Diamond Problem Example:**

```cpp
#include <iostream>

class Device {
public:
    int deviceID;
    Device() : deviceID(0) {
        std::cout << "Device constructor\n";
    }
    void powerOn() {
        std::cout << "Device powered on\n";
    }
};

class Scanner : public Device {
public:
    Scanner() {
        std::cout << "Scanner constructor\n";
    }
    void scan() {
        std::cout << "Scanning...\n";
    }
};

class Printer : public Device {
public:
    Printer() {
        std::cout << "Printer constructor\n";
    }
    void print() {
        std::cout << "Printing...\n";
    }
};

class Copier : public Scanner, public Printer {
public:
    Copier() {
        std::cout << "Copier constructor\n";
    }
};

int main() {
    Copier c;
    // c.deviceID = 5;  // ERROR: Ambiguous! Which deviceID?
    // c.powerOn();     // ERROR: Ambiguous! Which powerOn()?
    
    // Must specify which base class:
    c.Scanner::deviceID = 5;
    c.Printer::deviceID = 10;
    c.Scanner::powerOn();
    
    return 0;
}
```

When you create a Copier object, it contains two separate Device sub-objects (one through Scanner, one through Printer). This creates several problems: wasted memory from duplicate data members, ambiguity when accessing base class members, and the logical inconsistency of having two separate "Device" identities when conceptually there should only be one.

## Virtual Inheritance - The Solution

Virtual inheritance solves the diamond problem by ensuring that only one instance of the common base class exists, regardless of how many inheritance paths lead to it. When a class is inherited virtually, all derived classes share a single instance of that base class.

**Virtual Inheritance Example:**

```cpp
#include <iostream>

class Device {
public:
    int deviceID;
    Device(int id = 0) : deviceID(id) {
        std::cout << "Device constructor: ID = " << deviceID << "\n";
    }
    void powerOn() {
        std::cout << "Device " << deviceID << " powered on\n";
    }
};

// Virtual inheritance
class Scanner : virtual public Device {
public:
    Scanner() {
        std::cout << "Scanner constructor\n";
    }
    void scan() {
        std::cout << "Scanning...\n";
    }
};

class Printer : virtual public Device {
public:
    Printer() {
        std::cout << "Printer constructor\n";
    }
    void print() {
        std::cout << "Printing...\n";
    }
};

class Copier : public Scanner, public Printer {
public:
    Copier(int id) : Device(id) {  // Most derived class initializes virtual base
        std::cout << "Copier constructor\n";
    }
};

int main() {
    Copier c(42);
    
    // Now unambiguous - only ONE Device exists
    c.deviceID = 100;
    c.powerOn();
    
    std::cout << "Device ID: " << c.deviceID << "\n";
    c.scan();
    c.print();
    
    return 0;
}
```

**Key Points about Virtual Inheritance:**

With virtual inheritance, the most derived class (Copier in this example) is responsible for initializing the virtual base class (Device). This is different from normal inheritance where each intermediate class initializes its direct base. The virtual base class constructor is called first, before any of the intermediate classes, ensuring there's only one instance shared by all paths.

**Practical Real-World Example:**

```cpp
#include <iostream>
#include <string>

class Person {
protected:
    std::string name;
    int age;
public:
    Person(std::string n, int a) : name(n), age(a) {}
    void display() {
        std::cout << "Name: " << name << ", Age: " << age << "\n";
    }
};

class Student : virtual public Person {
protected:
    std::string studentID;
public:
    Student(std::string n, int a, std::string id) 
        : Person(n, a), studentID(id) {}
    void study() {
        std::cout << name << " is studying\n";
    }
};

class Employee : virtual public Person {
protected:
    double salary;
public:
    Employee(std::string n, int a, double s) 
        : Person(n, a), salary(s) {}
    void work() {
        std::cout << name << " is working\n";
    }
};

class TeachingAssistant : public Student, public Employee {
public:
    TeachingAssistant(std::string n, int a, std::string id, double s)
        : Person(n, a), Student(n, a, id), Employee(n, a, s) {}
    
    void assist() {
        std::cout << name << " is assisting with teaching\n";
    }
};

int main() {
    TeachingAssistant ta("Alice", 25, "S12345", 30000);
    ta.display();    // Only one Person, no ambiguity
    ta.study();
    ta.work();
    ta.assist();
    
    return 0;
}
```

In this teaching assistant example, a TA is both a student and an employee, but they're still fundamentally one person. Virtual inheritance ensures that there's only one Person object with one name and one age, which makes logical sense.

**Performance Considerations:**

Virtual inheritance does have a small performance cost because accessing members of a virtual base class typically requires an extra level of indirection (through a virtual base table pointer). However, this cost is usually negligible compared to the correctness and memory benefits it provides when solving the diamond problem.