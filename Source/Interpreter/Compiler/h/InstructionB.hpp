#pragma once
#include <iostream>
#include <vector>
#include "Instruction.hpp"

namespace anal
{
    class InstructionB {
        public:
            intrp::Instruction _instruction;
            std::vector<int8_t> _data;
        public:
        InstructionB(const intrp::Instruction instruction, const std::vector<int8_t>& data) {
            _instruction = instruction;
            _data = data;
        }
    };
} // namespace anal

