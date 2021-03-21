#pragma once
#include <iostream>
#include <filesystem>
#include "ScriptFile.hpp"

namespace anal {
    class Analyzer {
        private:
            scrf::ScriptFile* _scriptFile;
        public:
            void anayze(scrf::ScriptFile& scriptFile);
        private:
            int getTabs(std::string& line);        
    };
}