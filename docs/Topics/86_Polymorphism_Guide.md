# Polymorphic Usage of Base Class Pointers

## Short Answer: **YES!**

All three levels can be used as base class pointers. Each level provides a different interface to the same concrete object.

## Interface Access Matrix

```
┌─────────────────────────────────────────────────────────────────────┐
│ Pointer Type                    │ interpretScript │ Types │ Shell   │
├─────────────────────────────────┼─────────────────┼───────┼─────────┤
│ IScriptInterpreter*             │       ✓         │   ✗   │   ✗     │
│ IScriptInterpreterComm*         │       ✓         │   ✓   │   ✗     │
│ IScriptInterpreterShell*        │       ✓         │   ✓   │   ✓     │
└─────────────────────────────────────────────────────────────────────┘

Legend:
  ✓ = Accessible through this pointer type
  ✗ = Not accessible through this pointer type
  Types = SendFunc/RecvFunc type aliases
  Shell = listItems, listCommands, loadPlugin, executeCmd
```

## Code Example

```cpp
// Create a concrete implementation (Level 3)
auto concrete = std::make_unique<ConcreteInterpreter>(send, recv);

// Use as Level 1 pointer - minimal interface
IScriptInterpreter<ScriptType, DriverType>* level1 = concrete.get();
level1->interpretScript(script);  // ✓ Works
// level1->listItems();           // ✗ Compile error

// Use as Level 2 pointer - adds communication types
IScriptInterpreterComm<ScriptType, DriverType>* level2 = concrete.get();
level2->interpretScript(script);  // ✓ Works
using SendType = decltype(level2)::element_type::SendFunc;  // ✓ Works
// level2->listItems();           // ✗ Compile error

// Use as Level 3 pointer - full interface
IScriptInterpreterShell<ScriptType, DriverType>* level3 = concrete.get();
level3->interpretScript(script);  // ✓ Works
level3->listItems();              // ✓ Works
level3->listCommands();           // ✓ Works
```

## When to Use Each Level

### Use Level 1 Pointer When:
```cpp
// You only need script execution
void processScript(IScriptInterpreter<ScriptType, DriverType>* interpreter) {
    interpreter->interpretScript(myScript);
}
```
**Benefits:**
- Minimal coupling
- Callers don't need to know about communication or shell features
- Follows Interface Segregation Principle

### Use Level 2 Pointer When:
```cpp
// You need script execution and access to communication types
template<typename TDriver>
void setupInterpreter(
    IScriptInterpreterComm<ScriptType, TDriver>* interpreter,
    typename IScriptInterpreterComm<ScriptType, TDriver>::SendFunc send) 
{
    // Can work with SendFunc/RecvFunc types
    interpreter->interpretScript(myScript);
}
```
**Benefits:**
- Access to communication function types
- More flexibility than Level 1
- Still hides shell implementation details

### Use Level 3 Pointer When:
```cpp
// You need full interactive features
void runInteractiveShell(
    IScriptInterpreterShell<ScriptType, DriverType>* interpreter) 
{
    interpreter->listCommands();
    interpreter->executeCmd(userInput);
    interpreter->interpretScript(myScript);
}
```
**Benefits:**
- Full access to all features
- Interactive shell capabilities
- Complete interface

## Collections and Containers

```cpp
// Collection of Level 1 pointers - most flexible
std::vector<IScriptInterpreter<ScriptType, DriverType>*> interpreters;

// Can hold ANY interpreter (Level 1, 2, or 3 implementations)
interpreters.push_back(basicInterpreter);
interpreters.push_back(commInterpreter);
interpreters.push_back(shellInterpreter);

// Process using minimal interface
for (auto* interp : interpreters) {
    interp->interpretScript(script);  // Works for all!
}
```

## Dependency Inversion Principle

This design follows the **Dependency Inversion Principle**:

```
High-level modules should depend on abstractions, 
not concrete implementations.
```

**Example:**
```cpp
// BAD: Depends on concrete type
class ScriptProcessor {
    ConcreteInterpreter* m_interpreter;  // Tightly coupled!
};

// GOOD: Depends on minimal abstraction
class ScriptProcessor {
    IScriptInterpreter<ScriptType, DriverType>* m_interpreter;  // Loosely coupled!
};
```

## Virtual Function Resolution

All methods are resolved via virtual function table (vtable):

```
┌──────────────────┐
│ ConcreteInterp   │
├──────────────────┤
│ vtable ptr   ────┼──→ [interpretScript]
│ member data      │    [listItems]
└──────────────────┘    [listCommands]
                        [loadPlugin]
                        [executeCmd]

When called through:
- Level 1 pointer → Only sees interpretScript in vtable
- Level 2 pointer → Sees interpretScript (Level 1 inherits)
- Level 3 pointer → Sees all virtual functions
```

## Runtime Behavior

```cpp
// All point to the SAME object in memory
ConcreteInterpreter concrete;
IScriptInterpreter<S,D>* p1 = &concrete;
IScriptInterpreterComm<S,D>* p2 = &concrete;
IScriptInterpreterShell<S,D>* p3 = &concrete;

// All call the SAME virtual function implementation
p1->interpretScript(script);  // Calls ConcreteInterpreter::interpretScript
p2->interpretScript(script);  // Calls ConcreteInterpreter::interpretScript
p3->interpretScript(script);  // Calls ConcreteInterpreter::interpretScript

// But only p3 can see shell methods (compile-time check)
p3->listItems();  // ✓ Compiles and calls ConcreteInterpreter::listItems
p2->listItems();  // ✗ Compile error - method not in IScriptInterpreterComm
p1->listItems();  // ✗ Compile error - method not in IScriptInterpreter
```

## Factory Pattern

```cpp
// Factory returns minimal interface
class InterpreterFactory {
public:
    static std::unique_ptr<IScriptInterpreter<S,D>> create() {
        // Internally creates Level 3, returns as Level 1
        return std::make_unique<ConcreteInterpreter>(...);
    }
};

// Caller depends only on Level 1 interface
auto interp = InterpreterFactory::create();
interp->interpretScript(script);
```

## Casting

You can cast up and down the hierarchy:

```cpp
// Upcast (implicit, always safe)
ConcreteInterpreter concrete;
IScriptInterpreter<S,D>* base = &concrete;  // ✓ Implicit

// Downcast (explicit, runtime check recommended)
auto* shell = dynamic_cast<IScriptInterpreterShell<S,D>*>(base);
if (shell) {
    shell->listItems();  // Safe - object is actually a shell interpreter
}
```

## Summary

✅ **YES** - All levels can be used as base class pointers
✅ Each level exposes exactly the interface it defines
✅ Virtual dispatch works correctly through all levels
✅ Use the minimal level you need (Dependency Inversion)
✅ Collections can hold any level, limiting what's accessible

This is proper object-oriented polymorphism in action!
