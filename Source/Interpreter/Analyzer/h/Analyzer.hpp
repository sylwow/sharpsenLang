#pragma once
#include <iostream>
#include <regex>
#include <filesystem>
#include "ScriptFile.hpp"
#include "AnalyzerState.hpp"
#include "Instruction.hpp"
#include "BinaryInstructions.hpp"

namespace anal {
    class Analyzer {
        private:
            scrf::ScriptFile* _scriptFile;
            AnalyzerState _state = AnalyzerState::GlobalScope;
            size_t _lineCounter = 0;
            BinaryInstructions _instructions;
        public:
            void anayze(scrf::ScriptFile& scriptFile);
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