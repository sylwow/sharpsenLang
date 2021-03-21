#include "Analyzer.hpp"
#include <filesystem>

namespace anal {
    void Analyzer::anayze(scrf::ScriptFile& scriptFile) {
        _scriptFile = &scriptFile;
    }

    int getTabs(std::string& line) {
        int tabs = 0;
        for(int i = 0; i < line.size(); i++) {
            if(line.at(i) == '\t') {
                tabs++;
            }
        }
        return tabs;
    }
}