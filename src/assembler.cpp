#include <fmt/core.h>
#include <algorithm>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <emulator/assembler.hpp>
#include <emulator/instruction.hpp>
#include <emulator/types.hpp>
#include <iostream>
#include <span>
#include <stdexcept>
#include <string_view>
#include <userver/utils/from_string.hpp>

#include <userver/utils/text.hpp>
#include <userver/utils/text_light.hpp>

namespace emulator {

namespace {
constexpr MachineWord OPERAND_SHIFT = 0;
constexpr MachineWord WIDTH_SHIFT = 9;
constexpr MachineWord ADDR_MODE_SHIFT = 11;
constexpr MachineWord OPCODE_SHIFT = 13;
}  // namespace

Assembler::Assembler(std::span<const std::string_view> source_lines)
    : _source_lines(source_lines) {}

[[nodiscard]] Assembler::AssemblerOutput Assembler::Assemble() {
    AssemblerOutput output;

    auto instruction_begin = std::ranges::find(_source_lines, ".program");
    auto data_begin = std::ranges::find(_source_lines, ".data");

    size_t instruction_size = _source_lines.end() - instruction_begin;
    size_t data_size = _source_lines.end() - data_begin;

    if (data_size > instruction_size) {
        data_size -= instruction_size;
    } else {
        instruction_size -= data_size;
    }

    ParseDataSection(output, data_begin, data_size);
    ParseInstructionSection(output, instruction_begin, instruction_size);
    FillMachineCode(output);

    return output;
}

void emulator::Assembler::ParseDataSection(
    AssemblerOutput& output, Iterator begin, std::size_t size
) {
    auto data_span =
        _source_lines.subspan(begin + 1 - _source_lines.begin(), size - 1);
    std::size_t data_addr = 0;
    for (auto line : data_span) {
        auto line_view = userver::utils::text::TrimView(line);
        if (line_view.empty()) {
            continue;
        }
        const auto& elements = userver::utils::text::Split(line_view, " ");

        if (elements.empty()) {
            continue;
        }

        auto colon_pos = elements.front().find_first_of(":");
        if (!std::isalpha(elements.front().front())) {
            throw std::runtime_error{"Variable name must start with letter"};
        }
        std::string var_name = elements.front().substr(0, colon_pos);
        output.data_labels[var_name] = data_addr;
        std::cout << "VARNAME: " << var_name << '\n';

        auto width_code =
            kDataWidthToString.TryFindBySecond(elements[1]).value_or(DataWidth::kDW);

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

            std::uint64_t unsigned_data = static_cast<std::uint64_t>(int_data);
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
    }
}

void Assembler::ParseInstructionSection(
    AssemblerOutput& output, Iterator begin, std::size_t size
) {
    // Безопасный расчет диапазона строк (пропускаем строку заголовка секции,
    // например ".program")
    auto start_it = begin + 1;
    auto end_it = start_it + (size > 1 ? size - 1 : 0);

    std::uint16_t pc = 0;

    for (auto it = start_it; it != end_it; ++it) {
        std::string_view line = userver::utils::text::TrimView(*it);
        if (line.empty()) {
            continue;
        }

        const auto& elements = userver::utils::text::Split(line, " ");
        if (elements.empty()) continue;

        std::size_t bias = 0;

        // 1. Обработка метки (например, "loop_start:")
        if (elements.front().back() == ':') {
            std::string label =
                std::string(elements.front().substr(0, elements.front().size() - 1));
            output.labels[label] = pc;

            if (elements.size() == 1) {
                continue;  // Строка содержала только метку
            }
            ++bias;
        }

        if (bias >= elements.size()) continue;

        Instruction instruction;
        instruction.address = pc;
        instruction.raw = std::string(line);

        // 2. Обработка опкода
        auto opcode_opt = kOperationToString.TryFindBySecond(elements[bias]);
        if (!opcode_opt.has_value()) {
            throw std::runtime_error{fmt::format(
                "Error: the provided command:=={}==is unknown", elements[bias]
            )};
        }
        instruction.opcode = opcode_opt.value();
        ++bias;

        // 3. Обработка HLT (без операнда)
        if (instruction.opcode == Opcode::kHLT) {
            instruction.mode =
                AddressingMode::Immediate;  // Или специальный режим None
            instruction.operand = "";
            output.program.push_back(instruction);
            ++pc;
            continue;
        }

        if (bias >= elements.size()) {
            // TODO: Обработка ошибки: отсутствует операнд
            continue;
        }

        // 4. Проверка на явный спецификатор ширины (DB, DW, DD, DQ) перед операндом
        DataWidth inferred_width = DataWidth::kDB;  // Значение по умолчанию

        std::string_view width_token = elements[bias];
        if (width_token == "DB" || width_token == "DW" || width_token == "DD" ||
            width_token == "DQ") {
            // Замените на реальные значения вашего enum DataWidth
            if (width_token == "DB")
                inferred_width = DataWidth::kDB;
            else if (width_token == "DW")
                inferred_width = DataWidth::kDW;
            else if (width_token == "DD")
                inferred_width = DataWidth::kDD;
            else if (width_token == "DQ")
                inferred_width = DataWidth::kDQ;
            ++bias;
        }

        if (bias >= elements.size()) continue;

        // 5. Парсинг операнда и определение режима адресации
        std::string_view raw_operand = elements[bias];
        instruction.width =
            inferred_width;  // Устанавливаем ширину (явную или по умолчанию)

        if (raw_operand.starts_with('[') && raw_operand.ends_with(']')) {
            // Косвенная регистровая: [R1]
            instruction.mode = AddressingMode::IndirectRegister;
            instruction.operand =
                std::string(raw_operand.substr(1, raw_operand.size() - 2));  // "R1"

        } else if (raw_operand.starts_with('#')) {
            // Непосредственная: #4, #Array, #Array+4
            instruction.mode = AddressingMode::Immediate;
            instruction.operand = std::string(raw_operand.substr(1)
            );  // Убираем '#', получаем "4", "Array" или "Array+4"

        } else if (kRegisterNameString.TryFind(raw_operand).has_value()) {
            // Регистровая: R1
            instruction.mode = AddressingMode::Register;
            instruction.operand = std::string(raw_operand);

        } else {
            // Прямая адресация: Array
            instruction.mode = AddressingMode::Direct;
            instruction.operand = std::string(raw_operand);

            // Так как секция данных уже распарсена, мы можем сразу узнать ширину для
            // прямой адресации
            auto addr_it = output.data_labels.find(instruction.operand);
            if (addr_it != output.data_labels.end()) {
                auto width_it = output.data_width.find(addr_it->second);
                if (width_it != output.data_width.end()) {
                    instruction.width = width_it->second;
                }
            }
        }

        if ((instruction.opcode != Opcode::kLDA &&
             instruction.opcode != Opcode::kSTA &&
             instruction.opcode != Opcode::kJNZ &&
             instruction.opcode != Opcode::kHLT) &&
            (instruction.mode == AddressingMode::Direct ||
             instruction.mode == AddressingMode::IndirectRegister ||
             (instruction.mode == AddressingMode::Direct &&
              std::isalpha(instruction.operand.front())))) {
            throw std::runtime_error{fmt::format(
                "you cant get access to RAM without LDA or STA instruction, "
                "instruction: {}",
                line
            )};
        }

        output.program.push_back(instruction);
        ++pc;
    }
}

void Assembler::FillMachineCode(AssemblerOutput& output) const {
    for (auto& item : output.program) {
        MachineWord instruction = 0;
        instruction |= (static_cast<MachineWord>(item.opcode) << OPCODE_SHIFT);
        instruction |= (static_cast<MachineWord>(item.mode) << ADDR_MODE_SHIFT);

        MachineWord operand_value = 0;

        if (item.opcode == Opcode::kJNZ) {
            operand_value = output.labels.at(item.operand);
        } else if (item.opcode != Opcode::kHLT) {
            switch (item.mode) {
                case AddressingMode::Register:
                    operand_value = static_cast<MachineWord>(
                        kRegisterNameString.TryFindBySecond(item.operand).value()
                    );
                    break;

                case AddressingMode::IndirectRegister:
                    operand_value = static_cast<MachineWord>(
                        kRegisterNameString.TryFindBySecond(item.operand).value()
                    );
                    // Добавляем ширину данных для косвенной адресации
                    instruction |=
                        (static_cast<MachineWord>(
                             kDataWidthToMachineCode.TryFind(item.width).value()
                         )
                         << WIDTH_SHIFT);
                    break;

                case AddressingMode::Immediate: {
                    std::string op = item.operand;
                    size_t plus_pos = op.find('+');

                    if (plus_pos != std::string::npos) {
                        // Обработка синтаксиса "#Array+4"
                        std::string base_label = op.substr(0, plus_pos);
                        std::string offset_str = op.substr(plus_pos + 1);

                        std::uint16_t base_addr = output.data_labels.at(base_label);
                        int offset = userver::utils::FromString<int>(offset_str);
                        operand_value = base_addr + static_cast<MachineWord>(offset);

                        // Пытаемся получить ширину из базовой метки
                        auto width_it = output.data_width.find(base_addr);
                        if (width_it != output.data_width.end()) {
                            instruction |=
                                (static_cast<MachineWord>(
                                     kDataWidthToMachineCode
                                         .TryFind(width_it->second)
                                         .value()
                                 )
                                 << WIDTH_SHIFT);
                        } else {
                            instruction |=
                                (static_cast<MachineWord>(kDataWidthToMachineCode
                                                              .TryFind(item.width)
                                                              .value())
                                 << WIDTH_SHIFT);
                        }
                    } else if (std::isdigit(op.front()) ||
                               (op.front() == '-' && op.size() > 1 &&
                                std::isdigit(op[1]))) {
                        // Числовой литерал (например, "#4")
                        operand_value = static_cast<MachineWord>(
                            userver::utils::FromString<int>(op)
                        );
                        // Для литералов ширина может быть не указана или быть по
                        // умолчанию
                    } else {
                        // Чистая метка (например, "#Array")
                        operand_value = output.data_labels.at(op);
                        auto width_it = output.data_width.find(operand_value);
                        if (width_it != output.data_width.end()) {
                            instruction |=
                                (static_cast<MachineWord>(
                                     kDataWidthToMachineCode
                                         .TryFind(width_it->second)
                                         .value()
                                 )
                                 << WIDTH_SHIFT);
                        } else {
                            instruction |=
                                (static_cast<MachineWord>(kDataWidthToMachineCode
                                                              .TryFind(item.width)
                                                              .value())
                                 << WIDTH_SHIFT);
                        }
                    }
                    break;
                }

                case AddressingMode::Direct:
                    std::cout << "OPERAND: " << item.operand << ", width: "
                              << kDataWidthToString.TryFind(item.width).value()
                              << '\n';

                    size_t plus_pos = item.operand.find('+');
                    if(plus_pos == std::string::npos){

                    operand_value = output.data_labels.at(item.operand);
                    }else{
                        auto split = userver::utils::text::Split(item.operand, "+");

                        operand_value = output.data_labels.at(split.front()) + userver::utils::FromString<uint16_t>(split.back());
                    }
                    instruction |=
                        (static_cast<MachineWord>(
                             kDataWidthToMachineCode.TryFind(item.width).value()
                         )
                         << WIDTH_SHIFT);
                    break;
            }
        }

        instruction |= (operand_value << OPERAND_SHIFT);
        item.machine_code = instruction;
    }
}
}  // namespace emulator
