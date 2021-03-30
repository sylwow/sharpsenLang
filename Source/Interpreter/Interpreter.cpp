#include "Interpreter.hpp"
#include <filesystem>

namespace intrp {
    IInterpreter* createNewInterpreter() {
        return new Interpreter();
    }

    void Interpreter::load(std::filesystem::path& script) {
        _script = script;
    }

    void Interpreter::run() {
        _Compiler.compile(_script);
    }
}