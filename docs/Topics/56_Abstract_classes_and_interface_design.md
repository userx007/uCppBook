# Abstract Classes and Interface Design in C++

## Abstract Classes

An **abstract class** in C++ is a class that cannot be instantiated directly and is designed to serve as a base class for other classes. Abstract classes are created by declaring at least one **pure virtual function**, which is a virtual function with no implementation in the base class, denoted by `= 0`.

The primary purposes of abstract classes are:
- To define a common interface that derived classes must implement
- To provide a base for polymorphic behavior
- To enforce a contract that derived classes must follow

### Pure Virtual Functions

A pure virtual function is declared using the syntax:

```cpp
virtual return_type function_name(parameters) = 0;
```

Here's a basic example:

```cpp
class Shape {
public:
    // Pure virtual functions
    virtual double area() const = 0;
    virtual double perimeter() const = 0;
    
    // Regular virtual function with implementation
    virtual void display() const {
        std::cout << "Area: " << area() << ", Perimeter: " << perimeter() << std::endl;
    }
    
    virtual ~Shape() = default;  // Virtual destructor
};
```

Since `Shape` has pure virtual functions, you cannot create objects of type `Shape` directly. Any class inheriting from `Shape` must provide implementations for all pure virtual functions to become a concrete (instantiable) class.

### Concrete Derived Classes

```cpp
class Circle : public Shape {
private:
    double radius;
    
public:
    Circle(double r) : radius(r) {}
    
    // Must implement all pure virtual functions
    double area() const override {
        return 3.14159 * radius * radius;
    }
    
    double perimeter() const override {
        return 2 * 3.14159 * radius;
    }
};

class Rectangle : public Shape {
private:
    double width, height;
    
public:
    Rectangle(double w, double h) : width(w), height(h) {}
    
    double area() const override {
        return width * height;
    }
    
    double perimeter() const override {
        return 2 * (width + height);
    }
};
```

## Interface Design

In C++, an **interface** is typically implemented as an abstract class containing only pure virtual functions (and possibly a virtual destructor). This mimics the interface concept found in languages like Java and C#. An interface defines a contract that implementing classes must follow without providing any implementation details.

### Pure Interface Example

```cpp
class IDrawable {
public:
    virtual void draw() const = 0;
    virtual void setColor(const std::string& color) = 0;
    virtual ~IDrawable() = default;
};

class ISerializable {
public:
    virtual std::string serialize() const = 0;
    virtual void deserialize(const std::string& data) = 0;
    virtual ~ISerializable() = default;
};
```

By convention, interface names often start with 'I' to distinguish them from regular classes, though this is not required by the language.

### Multiple Interface Implementation

Unlike some languages, C++ supports multiple inheritance, which makes implementing multiple interfaces straightforward:

```cpp
class Document : public IDrawable, public ISerializable {
private:
    std::string content;
    std::string color;
    
public:
    Document(const std::string& text) : content(text), color("black") {}
    
    // Implementing IDrawable
    void draw() const override {
        std::cout << "Drawing document in " << color << ": " << content << std::endl;
    }
    
    void setColor(const std::string& c) override {
        color = c;
    }
    
    // Implementing ISerializable
    std::string serialize() const override {
        return "DOC:" + color + ":" + content;
    }
    
    void deserialize(const std::string& data) override {
        // Simple parsing logic
        size_t pos1 = data.find(':');
        size_t pos2 = data.find(':', pos1 + 1);
        color = data.substr(pos1 + 1, pos2 - pos1 - 1);
        content = data.substr(pos2 + 1);
    }
};
```

## Real-World Example: Database Access Layer

Here's a practical example showing how interfaces enable flexibility and testability:

```cpp
// Database interface
class IDatabase {
public:
    virtual bool connect(const std::string& connectionString) = 0;
    virtual void disconnect() = 0;
    virtual std::vector<std::string> query(const std::string& sql) = 0;
    virtual bool execute(const std::string& sql) = 0;
    virtual ~IDatabase() = default;
};

// MySQL implementation
class MySQLDatabase : public IDatabase {
public:
    bool connect(const std::string& connectionString) override {
        std::cout << "Connecting to MySQL: " << connectionString << std::endl;
        return true;
    }
    
    void disconnect() override {
        std::cout << "Disconnecting from MySQL" << std::endl;
    }
    
    std::vector<std::string> query(const std::string& sql) override {
        std::cout << "Executing MySQL query: " << sql << std::endl;
        return {"result1", "result2"};
    }
    
    bool execute(const std::string& sql) override {
        std::cout << "Executing MySQL command: " << sql << std::endl;
        return true;
    }
};

// PostgreSQL implementation
class PostgreSQLDatabase : public IDatabase {
public:
    bool connect(const std::string& connectionString) override {
        std::cout << "Connecting to PostgreSQL: " << connectionString << std::endl;
        return true;
    }
    
    void disconnect() override {
        std::cout << "Disconnecting from PostgreSQL" << std::endl;
    }
    
    std::vector<std::string> query(const std::string& sql) override {
        std::cout << "Executing PostgreSQL query: " << sql << std::endl;
        return {"pg_result1", "pg_result2"};
    }
    
    bool execute(const std::string& sql) override {
        std::cout << "Executing PostgreSQL command: " << sql << std::endl;
        return true;
    }
};

// Client code that depends on the interface, not implementation
class DataService {
private:
    IDatabase* database;
    
public:
    DataService(IDatabase* db) : database(db) {}
    
    void processData() {
        database->connect("server=localhost");
        auto results = database->query("SELECT * FROM users");
        for (const auto& result : results) {
            std::cout << "Processing: " << result << std::endl;
        }
        database->disconnect();
    }
};
```

This design allows you to easily swap database implementations:

```cpp
int main() {
    MySQLDatabase mysql;
    DataService service1(&mysql);
    service1.processData();
    
    PostgreSQLDatabase postgres;
    DataService service2(&postgres);
    service2.processData();
    
    return 0;
}
```

## Benefits of Abstract Classes and Interfaces

**Polymorphism**: You can write code that works with base class pointers/references while operating on derived class objects, enabling flexible and extensible designs.

**Dependency Inversion**: High-level modules depend on abstractions rather than concrete implementations, making code more maintainable and testable.

**Code Reusability**: Common behavior can be defined in abstract base classes, reducing duplication across derived classes.

**Testability**: Interfaces make it easy to create mock objects for unit testing without depending on actual implementations.

## Best Practices

When designing with abstract classes and interfaces, always provide a virtual destructor (even if it's defaulted) to ensure proper cleanup when deleting derived class objects through base class pointers. Keep interfaces focused and cohesive—follow the Interface Segregation Principle by creating small, specific interfaces rather than large, monolithic ones. Use pure interfaces (classes with only pure virtual functions) when defining contracts between components, and use abstract classes with some implementation when you want to share common code among derived classes. Finally, prefer composition over inheritance when possible, using interfaces to define capabilities rather than creating deep inheritance hierarchies.