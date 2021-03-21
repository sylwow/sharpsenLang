#pragma once
#include <filesystem>

namespace intrp {

    class IInterpreter {
        public:
            virtual void run() = 0; 
            virtual void load(std::filesystem::path& script) = 0;
    };

    IInterpreter* createNewInterpreter();
}