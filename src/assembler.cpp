#include <cctype>
#include <cstddef>
#include <cstdint>
#include <emulator/assembler.hpp>
#include <emulator/instruction.hpp>
#include <emulator/types.hpp>
#include <iostream>
#include <stdexcept>
#include <string_view>
#include <userver/utils/from_string.hpp>

#include <userver/utils/text.hpp>

namespace emulator {

namespace {
enum class Section { kData = 0, kProgram };

bool ends_with_colon(const std::string& s) { return !s.empty() && s.back() == ':'; }

constexpr int OPERAND_SHIFT = 0;
constexpr int WIDTH_SHIFT = 8;
constexpr int ADDR_MODE_SHIFT = 10;
constexpr int OPCODE_SHIFT = 12;

}  // namespace

[[nodiscard]] Assembler::AssemblerOutput Assembler::Assemble(
    std::span<const std::string_view> source_lines
) {
    AssemblerOutput output;
    std::uint16_t pc = 0;
    std::uint16_t data_addr = 0;
    Section section;

    for (std::string_view raw_line : source_lines) {
        if (raw_line.empty() || raw_line.front() == ';') {
            continue;
        }
        const auto& elements = userver::utils::text::Split(
            userver::utils::text::TrimView(raw_line), " "
        );

        if (elements.front() == ".data") {
            section = Section::kData;
            continue;
        }
        if (elements.front() == ".program") {
            section = Section::kProgram;
            continue;
        }
        switch (section) {
            case Section::kData: {
                auto colon_pos = elements.front().find_first_of(":");
                if (!std::isalpha(elements.front().front())) {
                    throw std::runtime_error{"Variable name must start with letter"};
                }
                std::string var_name = elements.front().substr(0, colon_pos);
                output.data_labels[var_name] = data_addr;
                std::cout << "VARNAME: " << var_name << '\n';

                auto width_code = kDataWidthToString.TryFindBySecond(elements[1])
                                      .value_or(DataWidth::kDW);

                output.data_width[data_addr] = width_code;

                std::vector<std::uint8_t> values;
                values.reserve((elements.size() - 2) * static_cast<int>(width_code));

                for (auto it = elements.begin() + 2; it != elements.end(); ++it) {
                    std::string_view element_view(*it);
                    if (!element_view.empty() && element_view.back() == ',') {
                        element_view.remove_suffix(1);
                    }

                    bool is_negative = element_view.front() == '-';

                    if (is_negative) {
                        element_view.remove_prefix(1);
                    }

                    std::int64_t int_data =
                        userver::utils::FromString<std::int64_t>(element_view);

                    std::uint64_t unsigned_data =
                        static_cast<std::uint64_t>(int_data);
                    if (is_negative) {
                        unsigned_data = ~unsigned_data + 1;
                    }

                    for (size_t i = 0; i < static_cast<size_t>(width_code); ++i) {
                        values.push_back(unsigned_data & 0xFF);
                        unsigned_data = unsigned_data >> 8;
                    }
                }

                output.data.push_back({var_name, values});
                data_addr += values.size();
                break;
            }
            case Section::kProgram: {
                int bias = 0;
                if (ends_with_colon(elements.front())) {
                    std::string label =
                        elements.front().substr(0, elements.front().size() - 1);
                    std::cout << label << '\n';
                    output.labels[label] = pc;
                    if (elements.size() == 1) {
                        continue;
                    }
                    ++bias;
                }

                Instruction instruction;
                instruction.address = pc;

                instruction.opcode =
                    kOperationToString.TryFindBySecond(elements[bias]).value();
                ++bias;
                AddressingMode mode = AddressingMode::Immediate;
                if (instruction.opcode != Opcode::kHLT) {
                    std::string_view operand = elements.back();

                    if (operand.size() > 2 && operand.front() == '[' &&
                        operand.back() == ']' &&
                        kRegisterNameString
                            .TryFindBySecond(operand.substr(1, operand.size() - 2))
                            .has_value()) {
                        mode = AddressingMode::IndirectRegister;
                    } else if (operand.front() == '#' && operand.size() > 1) {
                        mode = AddressingMode::Immediate;
                    } else {
                        if (kRegisterNameString.TryFind(operand).has_value()) {
                            mode = AddressingMode::Register;
                        } else {
                            mode = AddressingMode::Direct;
                        }
                    }

                    instruction.mode = mode;

                    switch (instruction.mode) {
                        case AddressingMode::Immediate:
                            instruction.operand =
                                std::string(operand.data() + 1, operand.size() - 1);
                            break;
                        case AddressingMode::Direct:
                        case AddressingMode::Register:
                            instruction.operand =
                                std::string(operand.data(), operand.size());
                            break;
                        case AddressingMode::IndirectRegister:
                            instruction.operand =
                                std::string(operand.data() + 1, operand.size() - 2);
                            break;
                    }
                }

                output.program.push_back(instruction);
                ++pc;
                break;
            }
        }
    }

    for (auto& item : output.program) {
        std::uint16_t instruction = 0;
        instruction |= (static_cast<uint16_t>(item.opcode) << OPCODE_SHIFT);
        instruction |= (static_cast<uint16_t>(item.mode) << ADDR_MODE_SHIFT);
        std::uint16_t operand = 0;
        if (item.opcode == Opcode::kJNZ) {
            operand = output.labels[item.operand];

            std::cout << "OPERAND: " << item.operand << ',' << operand << '\n';
        } else if (item.opcode != Opcode::kHLT) {
            DataWidth width;
            switch (item.mode) {
                case AddressingMode::Register:
                    operand = static_cast<std::uint16_t>(
                        kRegisterNameString.TryFindBySecond(item.operand).value()
                    );
                    break;
                case AddressingMode::IndirectRegister:
                    operand = static_cast<std::uint16_t>(
                        kRegisterNameString.TryFindBySecond(item.operand).value()
                    );
                    break;
                case AddressingMode::Immediate:
                    if (std::isdigit(item.operand.front())) {
                        operand = userver::utils::FromString<int8_t>(item.operand);
                    } else {
                        const auto& addr = output.data_labels.at(item.operand);
                        operand = addr;
                    }
                    break;
                case AddressingMode::Direct:
                    const auto& addr = output.data_labels.at(item.operand);
                    operand = addr;
                    width = output.data_width.at(addr);
                    instruction |=
                        (kDataWidthToMachineCode.TryFind(width).value()
                         << WIDTH_SHIFT);
                    break;
            }
        }
        instruction |= (operand << OPERAND_SHIFT);

        item.machine_code = instruction;
    }

    return output;
}

}  // namespace emulator
