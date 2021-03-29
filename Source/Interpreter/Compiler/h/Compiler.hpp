#pragma once
#include <iostream>
#include <regex>
#include <filesystem>
#include <unordered_map>
#include "ScriptFile.hpp"
#include "CompilerState.hpp"
#include "Instruction.hpp"
#include "BinaryInstructions.hpp"

namespace anal {
    class Compiler {
        private:
            scrf::ScriptFile* _scriptFile;
            CompilerState _state = CompilerState::GlobalScope;
            size_t _lineCounter = 0;
            BinaryInstructions _instructions;
            std::vector<std::unordered_map<std::string, int>> scopeNames;
        public:
            void compile(scrf::ScriptFile& scriptFile);
        private:
            int getTabs(const std::string& line);
            void analyzeLines();
            void analyzeLine(const std::string& line);
            void analyzeTabs(const size_t tabs);

            void makeInteger(const std::smatch& match);
            
            template<typename... Args>
            void printError(const Args... args) { (std::cout << ... << args) << std::endl;}       
    };
}