#pragma once
#include <iostream>
#include <filesystem>
#include "IInterpreter.hpp"
#include "Analyzer.hpp"
#include "ScriptFile.hpp"

namespace intrp {
    class Interpreter: public IInterpreter {
        private:
            anal::Analyzer _analyzer;
            scrf::ScriptFile _scriptFile;
            std::filesystem::path _script;
        public:
            void load(std::filesystem::path& script);
            void run();
    };
}