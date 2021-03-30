#pragma once
#include <iostream>
#include <filesystem>
#include "IInterpreter.hpp"
#include "Compiler.hpp"
#include "ScriptFile.hpp"

namespace intrp {
    class Interpreter: public IInterpreter {
        private:
            anal::Compiler _Compiler;
            
            std::filesystem::path _script;
        public:
            void load(std::filesystem::path& script);
            void run();
    };
}