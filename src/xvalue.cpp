#include <iostream>
#include <string>

struct Widget {
    std::string name;
    int value;

    Widget(std::string n, int v) : name(n), value(v) {}
};

Widget getWidget() {
    return Widget{"factory", 42};
}

int main() {
    // All of these produce xvalues:

    // 1. prvalue.member → xvalue
    auto&& a = getWidget().name;
    std::cout << "Type of a: " << typeid(decltype(a)).name() << '\n';

    // 2. std::move(prvalue).member → xvalue.member → xvalue
    auto&& b = std::move(getWidget()).name;
    std::cout << "Type of b: " << typeid(decltype(b)).name() << '\n';

    // 3. std::move(lvalue).member → xvalue.member → xvalue
    Widget w{"local", 99};
    auto&& c = std::move(w).name;
    std::cout << "Type of c: " << typeid(decltype(c)).name() << '\n';

    // All three are the same type: std::string&& (xvalue)
    std::cout << "\nAll are rvalue references (xvalues)\n";
}