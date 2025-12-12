// Function returning by value → prvalue
std::string getString();
Widget getWidget();

// ✗ DON'T: std::move on function return
auto&& x = std::move(getString());        // Redundant
auto&& y = std::move(getWidget()).name;   // Redundant

// ✓ DO: Just use directly
auto&& x = getString();         // Already movable
auto&& y = getWidget().name;    // Already xvalue

// ─────────────────────────────────────────

// Named variable → lvalue
std::string str;
Widget w;

// ✓ DO: std::move on lvalue when you want to move
auto&& a = std::move(str);      // Needed!
auto&& b = std::move(w).name;   // Needed!

// ✗ DON'T: Without std::move (if you want xvalue)
auto&& c = str;     // lvalue reference, not xvalue
auto&& d = w.name;  // lvalue reference, not xvalue