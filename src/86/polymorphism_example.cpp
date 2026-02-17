/**
 * @file polymorphism_example.cpp
 * @brief Demonstrating polymorphic usage of base class pointers at different levels
 */

#include "IScriptInterpreterShell.hpp"
#include <iostream>
#include <vector>
#include <memory>

// Example types
struct ScriptEntry {
    std::string command;
    std::string parameters;
};

class SerialDriver {};

//=============================================================================
// Concrete implementation at Level 3
//=============================================================================
class ConcreteInterpreter : public IScriptInterpreterShell<std::vector<ScriptEntry>, SerialDriver>
{
public:
    using SendFunc = IScriptInterpreterShell::SendFunc;
    using RecvFunc = IScriptInterpreterShell::RecvFunc;

    ConcreteInterpreter(SendFunc send, RecvFunc recv)
        : IScriptInterpreterShell(send, recv, 100, 4096)
    {
        std::cout << "ConcreteInterpreter constructed\n";
    }

    bool interpretScript(std::vector<ScriptEntry>& entries) override {
        std::cout << "ConcreteInterpreter::interpretScript() called\n";
        return true;
    }

    bool listItems() override {
        std::cout << "ConcreteInterpreter::listItems() called\n";
        return true;
    }

    bool listCommands() override {
        std::cout << "ConcreteInterpreter::listCommands() called\n";
        return true;
    }

    bool loadPlugin(const std::string& name) override {
        std::cout << "ConcreteInterpreter::loadPlugin(" << name << ") called\n";
        return true;
    }

    bool executeCmd(const std::string& cmd) override {
        std::cout << "ConcreteInterpreter::executeCmd(" << cmd << ") called\n";
        return true;
    }
};

//=============================================================================
// Polymorphic usage demonstration
//=============================================================================
int main()
{
    // Create send/recv functions
    auto send = [](uint32_t timeout, std::span<const uint8_t> data, 
                  std::shared_ptr<const SerialDriver> driver) {
        return ICommDriver::WriteResult{ICommDriver::Status::SUCCESS, data.size()};
    };

    auto recv = [](uint32_t timeout, std::span<uint8_t> buffer,
                  const ICommDriver::ReadOptions& options,
                  std::shared_ptr<const SerialDriver> driver) {
        return ICommDriver::ReadResult{ICommDriver::Status::SUCCESS, buffer.size(), false};
    };

    std::vector<ScriptEntry> script = {{"TEST", "data"}};

    std::cout << "=== Creating ConcreteInterpreter ===\n";
    auto concrete = std::make_unique<ConcreteInterpreter>(send, recv);
    std::cout << "\n";

    //=========================================================================
    // Level 1: Using IScriptInterpreter base pointer
    //=========================================================================
    std::cout << "=== Using Level 1 (IScriptInterpreter) pointer ===\n";
    {
        IScriptInterpreter<std::vector<ScriptEntry>, SerialDriver>* level1Ptr = concrete.get();
        
        // Can call: interpretScript()
        level1Ptr->interpretScript(script);
        
        // CANNOT call: SendFunc/RecvFunc types, shell methods
        // level1Ptr->listItems();  // ERROR: not in interface
        
        std::cout << "✓ Level 1 pointer works - interpretScript() accessible\n";
    }
    std::cout << "\n";

    //=========================================================================
    // Level 2: Using IScriptInterpreterComm base pointer
    //=========================================================================
    std::cout << "=== Using Level 2 (IScriptInterpreterComm) pointer ===\n";
    {
        IScriptInterpreterComm<std::vector<ScriptEntry>, SerialDriver>* level2Ptr = concrete.get();
        
        // Can call: interpretScript()
        level2Ptr->interpretScript(script);
        
        // Can access: SendFunc/RecvFunc types
        using SendType = IScriptInterpreterComm<std::vector<ScriptEntry>, SerialDriver>::SendFunc;
        using RecvType = IScriptInterpreterComm<std::vector<ScriptEntry>, SerialDriver>::RecvFunc;
        std::cout << "✓ SendFunc and RecvFunc types accessible\n";
        
        // CANNOT call: shell methods
        // level2Ptr->listItems();  // ERROR: not in interface
        
        std::cout << "✓ Level 2 pointer works - interpretScript() + types accessible\n";
    }
    std::cout << "\n";

    //=========================================================================
    // Level 3: Using IScriptInterpreterShell base pointer
    //=========================================================================
    std::cout << "=== Using Level 3 (IScriptInterpreterShell) pointer ===\n";
    {
        IScriptInterpreterShell<std::vector<ScriptEntry>, SerialDriver>* level3Ptr = concrete.get();
        
        // Can call: ALL methods
        level3Ptr->interpretScript(script);
        level3Ptr->listItems();
        level3Ptr->listCommands();
        level3Ptr->loadPlugin("test_plugin");
        level3Ptr->executeCmd("SEND DATA");
        
        std::cout << "✓ Level 3 pointer works - ALL methods accessible\n";
    }
    std::cout << "\n";

    //=========================================================================
    // Practical use case: Collections of different level pointers
    //=========================================================================
    std::cout << "=== Practical use case: Collections ===\n";
    
    // Collection of Level 1 pointers - minimal interface
    std::vector<IScriptInterpreter<std::vector<ScriptEntry>, SerialDriver>*> basicInterpreters;
    basicInterpreters.push_back(concrete.get());
    
    std::cout << "Processing with Level 1 collection:\n";
    for (auto* interp : basicInterpreters) {
        interp->interpretScript(script);
    }
    std::cout << "\n";
    
    // Collection of Level 2 pointers - with communication
    std::vector<IScriptInterpreterComm<std::vector<ScriptEntry>, SerialDriver>*> commInterpreters;
    commInterpreters.push_back(concrete.get());
    
    std::cout << "Processing with Level 2 collection:\n";
    for (auto* interp : commInterpreters) {
        interp->interpretScript(script);
        // Types available here
    }
    std::cout << "\n";
    
    // Collection of Level 3 pointers - full featured
    std::vector<IScriptInterpreterShell<std::vector<ScriptEntry>, SerialDriver>*> shellInterpreters;
    shellInterpreters.push_back(concrete.get());
    
    std::cout << "Processing with Level 3 collection:\n";
    for (auto* interp : shellInterpreters) {
        interp->interpretScript(script);
        interp->listCommands();
    }
    std::cout << "\n";

    //=========================================================================
    // Factory pattern example
    //=========================================================================
    std::cout << "=== Factory pattern example ===\n";
    
    // Factory returns Level 1 pointer - minimal coupling
    auto createBasicInterpreter = []() -> std::unique_ptr<IScriptInterpreter<std::vector<ScriptEntry>, SerialDriver>> {
        auto send = [](uint32_t timeout, std::span<const uint8_t> data, 
                      std::shared_ptr<const SerialDriver> driver) {
            return ICommDriver::WriteResult{ICommDriver::Status::SUCCESS, data.size()};
        };
        auto recv = [](uint32_t timeout, std::span<uint8_t> buffer,
                      const ICommDriver::ReadOptions& options,
                      std::shared_ptr<const SerialDriver> driver) {
            return ICommDriver::ReadResult{ICommDriver::Status::SUCCESS, buffer.size(), false};
        };
        return std::make_unique<ConcreteInterpreter>(send, recv);
    };
    
    auto interpreter = createBasicInterpreter();
    interpreter->interpretScript(script);
    std::cout << "✓ Factory pattern works with Level 1 pointer\n";

    return 0;
}

/*
OUTPUT SUMMARY:
===============

YES, all levels can be used as base class pointers!

Interface access per level:
---------------------------
Level 1 (IScriptInterpreter*):
  ✓ interpretScript()
  ✗ SendFunc/RecvFunc types
  ✗ Shell methods

Level 2 (IScriptInterpreterComm*):
  ✓ interpretScript()
  ✓ SendFunc/RecvFunc types
  ✗ Shell methods

Level 3 (IScriptInterpreterShell*):
  ✓ interpretScript()
  ✓ SendFunc/RecvFunc types
  ✓ Shell methods (listItems, listCommands, loadPlugin, executeCmd)

Use cases:
----------
- Level 1 pointer: When you only need script execution (minimal coupling)
- Level 2 pointer: When you need script execution + communication types
- Level 3 pointer: When you need full shell functionality

This follows the Dependency Inversion Principle - depend on the
minimal interface you actually need!
*/
