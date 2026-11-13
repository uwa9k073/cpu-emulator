#pragma once

#include <cctype>
#include <cstddef>
#include <cstdint>
#include <map>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>
#include <userver/utils/from_string.hpp>
#include <userver/utils/text.hpp>

#include <emulator/instruction.hpp>
#include <emulator/types.hpp>

namespace cpu_emulator::core {
class Assembler {
public:
    explicit Assembler(std::span<const std::string_view> source_lines);

    using LabelMap = std::unordered_map<std::string, std::uint16_t>;

    struct AssemblerOutput {
        std::vector<Instruction> program;  // инструкции (PC 0, 1, 2, 3...)
        std::vector<std::pair<std::string, std::vector<std::uint8_t>>>
            data;              // переменные
        LabelMap labels;       // программные метки
        LabelMap data_labels;  // адреса переменных
        std::map<std::uint16_t, DataWidth> data_width;
    };

    [[nodiscard]] AssemblerOutput Assemble();

private:
    std::span<const std::string_view> _source_lines;

    using Iterator = decltype(_source_lines.begin());

    void ParseDataSection(AssemblerOutput& output, Iterator begin, std::size_t size);
    void ParseInstructionSection(
        AssemblerOutput& output, Iterator begin, std::size_t size
    );

    void FillMachineCode(AssemblerOutput& output) const;
};

}  // namespace cpu_emulator::core
