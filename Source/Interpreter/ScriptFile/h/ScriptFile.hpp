#pragma once
#include <iostream>
#include <filesystem>
#include <vector>

namespace scrf {
    class ScriptFile {
        private:
            std::vector<std::string> _lines;
        public:
            ScriptFile() {}
            void load(std::filesystem::path scriptPath);
            size_t getLinesNumber() { return _lines.size(); };
            std::string& getLine(size_t index) { return _lines.at(index); }
    };
}