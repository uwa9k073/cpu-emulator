module;
#include <iostream>

#include <cctype>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

export module assembler;

import types;
import instruction;

export namespace cpu {

struct AssemblerOutput {
    std::vector<Instruction> program;  // инструкции (PC 0, 1, 2, 3...)
    std::vector<std::pair<std::string, std::vector<std::int16_t>>>
        data;                                               // переменные
    std::unordered_map<std::string, std::uint16_t> labels;  // программные метки
    std::unordered_map<std::string, std::uint16_t> data_labels;  // адреса переменных
};

namespace {
enum class Section { kData = 0, kProgram };
}

class Assembler {
public:
    [[nodiscard]] AssemblerOutput assemble(
        const std::vector<std::string>& source_lines
    ) {
        AssemblerOutput output;
        std::uint16_t pc = 0;
        std::uint16_t data_addr = 0;
        Section section;

        for (const auto& raw_line : source_lines) {
            std::string line = trim(raw_line);
            if (line.empty() || line.starts_with(";")) {
                continue;
            }
            std::cout << line << '\n';
            if (line == ".data" || line == "DATA SECTION") {
                section = Section::kData;
                continue;
            }
            if (line == ".program") {
                section = Section::kProgram;
                continue;
            }
            switch (section) {
                case Section::kData: {
                    auto colon_pos = line.find_first_of(":");
                    std::string var_name = trim(line.substr(0, colon_pos));
                    output.data_labels[var_name] = data_addr;
                    line = trim(line.substr(colon_pos + 1));

                    if (line.starts_with("DW")) {
                        // DW означает "Define Word" (32-бит число или массив)
                        std::string values_str = trim(line.substr(2));
                        std::vector<std::int16_t> values =
                            parse_data_values(values_str);
                        for (auto val : values) {
                            output.data.push_back({"", {val}});
                            ++data_addr;
                        }
                    }
                }
                break;
                case Section::kProgram: {
                    if (ends_with_colon(line)) {
                        std::string label = trim(line.substr(0, line.size() - 1));
                        output.labels[label] = pc;
                        continue;
                    }

                    output.program.push_back({.operand = "", .raw = line, .address = pc}
                    );
                    ++pc;
                }
                break;
            }
        }

        for (auto& item : output.program) {
            auto parsed = split_mnemonic_operand(item.raw);
            auto [machine_code, addr_mode] =
                encode_instruction(item.opcode, item.operand, output.labels);
            item.opcode = parsed.first;
            item.operand = parsed.second;
            item.machine_code = machine_code;
            item.mode = addr_mode;
        }

        return output;
    }

    static std::pair<Opcode, std::string> split_mnemonic_operand(
        const std::string& text
    ) {
        std::string line = trim(text);
        if (line.empty()) {
            return {Opcode::NOP, ""};
        }

        const std::size_t pos = line.find(' ');
        if (pos == std::string::npos) {
            return {InstructionSet::from_string(line), ""};
        }

        auto mnemonic =
            InstructionSet::from_string(to_upper(trim(line.substr(0, pos))));
        std::string operand = trim(line.substr(pos + 1));
        return {mnemonic, operand};
    }

    static std::pair<std::uint16_t, AddressingMode> encode_instruction(
        Opcode mnemonic,
        const std::string& operand,
        const std::unordered_map<std::string, std::uint16_t>& labels
    ) {
        switch (mnemonic) {
            case Opcode::NOP:
                return {
                    static_cast<std::uint16_t>(Opcode::NOP) << 8,
                    AddressingMode::Immediate
                };
            case Opcode::LDA:
                return encode_load_like(Opcode::LDA, operand, labels);
            case Opcode::STA:
                return encode_store_like(Opcode::STA, operand, labels);
            case Opcode::ADD:
                return encode_alu_like(Opcode::ADD, operand, labels);
            case Opcode::SUB:
                return encode_alu_like(Opcode::SUB, operand, labels);
            case Opcode::MUL:
                return encode_alu_like(Opcode::MUL, operand, labels);
            case Opcode::DIV:
                return encode_alu_like(Opcode::DIV, operand, labels);
            case Opcode::INC:
                return encode_inc_dec(Opcode::INC, operand);
            case Opcode::DEC:
                encode_inc_dec(Opcode::DEC, operand);
            case Opcode::NEG:
                return encode_alu_like(Opcode::NEG, operand, labels);
            case Opcode::CMP:
                return encode_alu_like(Opcode::CMP, operand, labels);
            case Opcode::JMP:
                return encode_jump_like(Opcode::JMP, operand, labels);
            case Opcode::JZ:
                return encode_jump_like(Opcode::JZ, operand, labels);
            case Opcode::JNZ:
                return encode_jump_like(Opcode::JNZ, operand, labels);
            case Opcode::JC:
                return encode_jump_like(Opcode::JC, operand, labels);
            case Opcode::JNC:
                return encode_jump_like(Opcode::JNC, operand, labels);
            case Opcode::JS:
                return encode_jump_like(Opcode::JS, operand, labels);
            case Opcode::JNS:
                return encode_jump_like(Opcode::JNS, operand, labels);
            case Opcode::JGT:
                return encode_jump_like(Opcode::JGT, operand, labels);
            case Opcode::JGE:
                return encode_jump_like(Opcode::JGE, operand, labels);
            case Opcode::JLT:
                return encode_jump_like(Opcode::JLT, operand, labels);
            case Opcode::JLE:
                return encode_jump_like(Opcode::JLE, operand, labels);
            case Opcode::HLT:
                return static_cast<std::uint16_t>(Opcode::HLT) << 8;
            case Opcode::SHL:
                return encode_shift_like(Opcode::SHL, operand);
            case Opcode::ADDL:
                return encode_long_arith_like(Opcode::ADDL, operand);
            case Opcode::SUBL:
                return encode_long_arith_like(Opcode::SUBL, operand);
            case Opcode::MULL:
                return encode_long_arith_like(Opcode::MULL, operand);
            case Opcode::CMP32:
                return encode_long_arith_like(Opcode::CMP32, operand);
            case Opcode::LDS:
                return encode_load_like(Opcode::LDS, operand, labels);
            case Opcode::STS:
                return encode_store_like(Opcode::STS, operand, labels);
            default:
                throw std::runtime_error(
                    "Unknown mnemonic: " +
                    std::string{InstructionSet::to_string(mnemonic)}
                );
        }
    }

private:
    static std::string trim(const std::string& s) {
        std::size_t begin = 0;
        while (begin < s.size() && std::isspace(static_cast<unsigned char>(s[begin]))
        ) {
            ++begin;
        }

        std::size_t end = s.size();
        while (end > begin && std::isspace(static_cast<unsigned char>(s[end - 1]))) {
            --end;
        }

        return s.substr(begin, end - begin);
    }

    static std::string to_upper(const std::string& s) {
        std::string out = s;
        for (char& ch : out) {
            ch = static_cast<char>(std::toupper(static_cast<unsigned char>(ch)));
        }
        return out;
    }

    static bool ends_with_colon(const std::string& s) {
        return !s.empty() && s.back() == ':';
    }

    static std::pair<std::uint16_t, AddressingMode> encode_load_like(
        Opcode op,
        const std::string& operand,
        const std::unordered_map<std::string, std::uint16_t>& labels
    ) {
        std::uint16_t value = 0;

        if (operand.empty()) {
            return {
                (static_cast<std::uint16_t>(op) << 8) | 0, AddressingMode::Immediate
            };
        }

        if (operand.starts_with("#")) {
            const std::string number = trim(operand.substr(1));
            value = static_cast<std::uint16_t>(std::stoi(number));
            return {
                (static_cast<std::uint16_t>(op) << 8) | value,
                AddressingMode::Immediate
            };
        }

        if (auto it = labels.find(operand); it != labels.end()) {
            value = it->second;
            return (static_cast<std::uint16_t>(op) << 8) | value;
        }

        if (is_register_name(operand)) {
            value = register_id(operand);
            return {
                (static_cast<std::uint16_t>(op) << 8) | value,
                AddressingMode::Register
            };
        }

        if (operand.starts_with("(") && operand.ends_with(")")) {
            std::string inner = operand.substr(1, operand.size() - 2);
            value = register_id(inner);
            return (static_cast<std::uint16_t>(op) << 8) | value;
        }

        return (static_cast<std::uint16_t>(op) << 8) | 0;
    }

    static std::uint16_t encode_store_like(
        Opcode op,
        const std::string& operand,
        const std::unordered_map<std::string, std::uint16_t>& labels
    ) {
        if (operand.empty()) {
            return (static_cast<std::uint16_t>(op) << 8) | 0;
        }

        if (auto it = labels.find(operand); it != labels.end()) {
            return (static_cast<std::uint16_t>(op) << 8) | it->second;
        }

        if (is_register_name(operand)) {
            return (static_cast<std::uint16_t>(op) << 8) | register_id(operand);
        }

        if (operand.starts_with("(") && operand.ends_with(")")) {
            std::string inner = operand.substr(1, operand.size() - 2);
            return (static_cast<std::uint16_t>(op) << 8) | register_id(inner);
        }

        return (static_cast<std::uint16_t>(op) << 8) | 0;
    }

    static std::uint16_t encode_alu_like(
        Opcode op,
        const std::string& operand,
        const std::unordered_map<std::string, std::uint16_t>& labels
    ) {
        std::uint16_t value = 0;

        if (operand.empty()) {
            return (static_cast<std::uint16_t>(op) << 8) | 0;
        }

        if (operand.starts_with("#")) {
            value = static_cast<std::uint16_t>(std::stoi(trim(operand.substr(1))));
            return (static_cast<std::uint16_t>(op) << 8) | value;
        }

        if (is_register_name(operand)) {
            value = register_id(operand);
            return (static_cast<std::uint16_t>(op) << 8) | value;
        }

        if (auto it = labels.find(operand); it != labels.end()) {
            value = it->second;
            return (static_cast<std::uint16_t>(op) << 8) | value;
        }

        if (operand.starts_with("(") && operand.ends_with(")")) {
            std::string inner = operand.substr(1, operand.size() - 2);
            value = register_id(inner);
            return (static_cast<std::uint16_t>(op) << 8) | value;
        }

        return (static_cast<std::uint16_t>(op) << 8) | 0;
    }

    static std::uint16_t encode_jump_like(
        Opcode op,
        const std::string& operand,
        const std::unordered_map<std::string, std::uint16_t>& labels
    ) {
        if (auto it = labels.find(operand); it != labels.end()) {
            return (static_cast<std::uint16_t>(op) << 8) | it->second;
        }

        if (operand.starts_with("#")) {
            return (static_cast<std::uint16_t>(op) << 8) |
                   static_cast<std::uint16_t>(std::stoi(trim(operand.substr(1))));
        }

        return (static_cast<std::uint16_t>(op) << 8) | 0;
    }

    static std::uint16_t encode_inc_dec(Opcode op, const std::string& operand) {
        if (operand.empty()) {
            return (static_cast<std::uint16_t>(op) << 8) | 0;
        }

        if (is_register_name(operand)) {
            return (static_cast<std::uint16_t>(op) << 8) | register_id(operand);
        }

        return (static_cast<std::uint16_t>(op) << 8) | 0;
    }

    static std::uint16_t encode_shift_like(Opcode op, const std::string& operand) {
        if (operand.empty()) {
            return (static_cast<std::uint16_t>(op) << 8) | 0;
        }

        std::string number = operand;
        if (number.starts_with("#")) {
            number = trim(number.substr(1));
        }

        return (static_cast<std::uint16_t>(op) << 8) |
               static_cast<std::uint16_t>(std::stoi(number));
    }

    static std::uint16_t encode_long_arith_like(
        Opcode op, const std::string& operand
    ) {
        if (operand.empty()) {
            return (static_cast<std::uint16_t>(op) << 8) | 0;
        }

        if (is_register_name(operand)) {
            return (static_cast<std::uint16_t>(op) << 8) | register_id(operand);
        }

        return (static_cast<std::uint16_t>(op) << 8) | 0;
    }

    static bool is_register_name(const std::string& token) {
        const std::string upper = to_upper(token);
        return upper == "A" || upper == "R0" || upper == "R1" || upper == "R2" ||
               upper == "R3" || upper == "R4" || upper == "R5" || upper == "R6" ||
               upper == "R7" || upper == "H" || upper == "L";
    }

    static std::uint16_t register_id(const std::string& token) {
        const std::string upper = to_upper(token);

        if (upper == "A") return static_cast<std::uint16_t>(Register::A);
        if (upper == "R0") return static_cast<std::uint16_t>(Register::R0);
        if (upper == "R1") return static_cast<std::uint16_t>(Register::R1);
        if (upper == "R2") return static_cast<std::uint16_t>(Register::R2);
        if (upper == "R3") return static_cast<std::uint16_t>(Register::R3);
        if (upper == "R4") return static_cast<std::uint16_t>(Register::R4);
        if (upper == "R5") return static_cast<std::uint16_t>(Register::R5);
        if (upper == "R6") return static_cast<std::uint16_t>(Register::R6);
        if (upper == "R7") return static_cast<std::uint16_t>(Register::R7);
        if (upper == "H") return static_cast<std::uint16_t>(Register::H);
        if (upper == "L") return static_cast<std::uint16_t>(Register::L);

        return 0;
    }
    static std::vector<std::int16_t> parse_data_values(const std::string& values_str
    ) {
        std::vector<std::int16_t> result;
        std::string current;

        for (char ch : values_str) {
            if (ch == ',') {
                std::string trimmed = trim(current);
                if (!trimmed.empty()) {
                    result.push_back(static_cast<std::int16_t>(std::stoi(trimmed)));
                }
                current.clear();
            } else {
                current += ch;
            }
        }

        // Последний элемент
        std::string trimmed = trim(current);
        if (!trimmed.empty()) {
            result.push_back(static_cast<std::int16_t>(std::stoi(trimmed)));
        }

        return result;
    }
};

}  // namespace cpu
