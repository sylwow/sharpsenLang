#include "Analyzer.hpp"
#include "SyntaxError.hpp"
#include "InstructionStatements.hpp"
#include <filesystem>
#include <regex>

namespace anal {
    void Analyzer::anayze(scrf::ScriptFile& scriptFile) {
        _scriptFile = &scriptFile;
        analyzeLines();
    }

    void Analyzer::analyzeLines() {
        _state = AnalyzerState::GlobalScope;
        auto len = _scriptFile->getLinesNumber();
        try {
            for(_lineCounter = 0; _lineCounter < len; ++_lineCounter) {
                auto& line = _scriptFile->getLine(_lineCounter);
                int tabs = getTabs(line);
                analyzeTabs(tabs);
                const auto& rawLine = line.substr(tabs);
                analyzeLine(rawLine);
            }
        } catch (SyntaxError& e) {
            printError("SyntaxError at Line: ", _lineCounter, " | ", e.what());
        }
    }

    int Analyzer::getTabs(const std::string& line) {
        int tabs = 0;
        for(int i = 0; i < line.size(); i++) {
            if(line.at(i) == '\t') {
                tabs++;
            }
        }
        return tabs;
    }

    void Analyzer::analyzeTabs(const size_t tabs) {
        switch (_state)
        {
            case AnalyzerState::GlobalScope:
                if (tabs != 0) 
                    throw SyntaxError("Do not insert tabs in global scope");
                break;
            
            default:
                break;
        }
    }

    void Analyzer::analyzeLine(const std::string& line) {
        std::smatch cm;
        if (std::regex_match(line, cm, std::regex(NEW_INTEGER))) {
            makeInteger(cm);
        }
    }

    void Analyzer::makeInteger(const std::smatch& match) {
        int32_t integer;
        auto strInt = match[2].str();
        try {
            integer = std::stoi( strInt );
        } catch(...) {
            throw SyntaxError(std::string("Cound not parse number value: ") + strInt);
        }
        std::vector<int8_t> buff = {
            (int8_t)integer,
            (int8_t)(integer >> 8),
            (int8_t)(integer >> 16),
            (int8_t)(integer >> 24)
        };
        _instructions.addInstruction(intrp::Instruction::AssignNumber, buff);
    }
}