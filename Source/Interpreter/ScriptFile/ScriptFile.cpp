#include "ScriptFile.hpp"
#include <fstream>
#include <string>

namespace scrf {
    void ScriptFile::load(std::filesystem::path scriptPath) {
        std::ifstream file(scriptPath);
        std::string str; 
        while (std::getline(file, str)) {
            _lines.push_back(str);
        }
    }
}