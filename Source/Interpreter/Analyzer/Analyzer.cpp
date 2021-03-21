#include "Analyzer.hpp"
#include <filesystem>

namespace anal {
    void Analyzer::anayze(scrf::ScriptFile& scriptFile) {
        _scriptFile = &scriptFile;
    }
}