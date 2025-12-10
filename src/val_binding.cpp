#include <iostream>
#include <string>
#include <type_traits>

struct Widget {
    std::string name;
    int value;
};

Widget getWidget() { return Widget{"test", 42}; }

template<typename T>
void analyzeBinding(T&& val, const char* desc) {
    std::cout << desc << ":\n";
    std::cout << "  Binds as: "
              << (std::is_lvalue_reference_v<T> ? "lvalue" : "rvalue")
              << '\n';
}

int main() {
    Widget w{"local", 99};

    std::cout << "=== On lvalues ===\n";
    analyzeBinding(w.name, "w.name");                    // lvalue
    analyzeBinding(std::move(w).name, "std::move(w).name");  // xvalue - NEEDED!

    std::cout << "\n=== On prvalues ===\n";
    analyzeBinding(getWidget().name, "getWidget().name");  // xvalue
    analyzeBinding(std::move(getWidget()).name, "std::move(getWidget()).name");  // xvalue - REDUNDANT!
}

/*
```

**Output:**
```
=== On lvalues ===
w.name:
  Binds as: lvalue
std::move(w).name:
  Binds as: rvalue

=== On prvalues ===
getWidget().name:
  Binds as: rvalue
std::move(getWidget()).name:
  Binds as: rvalue

*/