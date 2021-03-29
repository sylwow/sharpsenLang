#pragma once
#include <iostream>
#include "InstructionB.hpp"

namespace anal {
    class BinaryInstructions {
        public:
        void addInstruction(const intrp::Instruction instruction, const std::vector<int8_t>& data) {
            _instructions.push_back(InstructionB(instruction, data));
        }
        private:
        std::vector<InstructionB> _instructions;
    };
}