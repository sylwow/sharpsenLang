#include "Compiler.hpp"
#include "SyntaxError.hpp"
#include "InstructionStatements.hpp"
#include <filesystem>
#include <regex>
#include <fstream>

namespace anal {
    void Compiler::compile(std::filesystem::path& scriptFile) {
        auto& byteFile = scriptFile.replace_extension(".sbyc");
        if (std::filesystem::exists(byteFile)) {
            loadByteCode(byteFile);
        } else {
            scopeNames.push_back({});
            analyzeLines();
        }
    }

    void Compiler::loadByteCode(std::filesystem::path& byteFilePath) {
         std::ifstream byteFile(byteFilePath, std::ios::binary);

        std::vector<char> bytes(
         (std::istreambuf_iterator<char>(byteFile)),
         (std::istreambuf_iterator<char>()));

        byteFile.close();
    }

    void Compiler::analyzeLines() {
        _state = CompilerState::GlobalScope;
        auto len = _scriptFile.getLinesNumber();
        try {
            for(_lineCounter = 0; _lineCounter < len; ++_lineCounter) {
                auto& line = _scriptFile.getLine(_lineCounter);
                int tabs = getTabs(line);
                analyzeTabs(tabs);
                const auto& rawLine = line.substr(tabs);
                analyzeLine(rawLine);
            }
        } catch (SyntaxError& e) {
            printError("SyntaxError at Line: ", _lineCounter, " | ", e.what());
        }
    }

    int Compiler::getTabs(const std::string& line) {
        int tabs = 0;
        for(int i = 0; i < line.size(); i++) {
            if(line.at(i) == '\t') {
                tabs++;
            }
        }
        return tabs;
    }

    void Compiler::analyzeTabs(const size_t tabs) {
        switch (_state)
        {
            case CompilerState::GlobalScope:
                if (tabs != 0) 
                    throw SyntaxError("Do not insert tabs in global scope");
                break;
            
            default:
                break;
        }
    }

    void Compiler::analyzeLine(const std::string& line) {
        std::smatch cm;
        if (std::regex_match(line, cm, std::regex(NEW_INTEGER))) {
            makeInteger(cm);
        }
    }

    void Compiler::makeInteger(const std::smatch& match) {
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