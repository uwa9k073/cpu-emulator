#pragma once

#include <cctype>
#include <cstdint>
#include <map>
#include <span>
#include <string>
#include <unordered_map>
#include <userver/utils/from_string.hpp>
#include <userver/utils/text.hpp>

#include <emulator/instruction.hpp>
#include <emulator/types.hpp>

namespace emulator {
class Assembler {
public:
    struct AssemblerOutput {
        std::vector<Instruction> program;  // инструкции (PC 0, 1, 2, 3...)
        std::vector<std::pair<std::string, std::vector<std::uint8_t>>>
            data;  // переменные
        std::unordered_map<std::string, std::uint16_t> labels;  // программные метки
        std::unordered_map<std::string, std::uint16_t>
            data_labels;  // адреса переменных
        std::map<std::uint16_t, DataWidth> data_width;
    };
    [[nodiscard]] AssemblerOutput Assemble(
        std::span<const std::string_view> source_lines
    );
};

}  // namespace emulator
