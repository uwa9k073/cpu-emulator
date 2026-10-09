module;

#include <cctype>
#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

export module assembler;

import instruction;
import types;

export namespace cpu {

struct AssembledLine {
    std::string mnemonic;
    std::string operand;
    std::string raw;
    std::uint16_t machine_code = 0;
    std::uint16_t address = 0;
};

class Assembler {
public:
    [[nodiscard]] std::vector<AssembledLine> assemble(const std::vector<std::string>& source_lines) {
        std::unordered_map<std::string, std::uint16_t> labels;
        std::vector<AssembledLine> output;

        std::uint16_t pc = 0;
        for (const auto& raw_line : source_lines) {
            std::string line = trim(raw_line);
            if (line.empty() || line.starts_with(";")) {
                continue;
            }

            if (ends_with_colon(line)) {
                std::string label = trim(line.substr(0, line.size() - 1));
                labels[label] = pc;
                continue;
            }

            output.push_back({"", "", line, 0, pc});
            ++pc;
        }

        for (auto& item : output) {
            auto parsed = split_mnemonic_operand(item.raw);
            item.mnemonic = parsed.first;
            item.operand = parsed.second;
            item.machine_code = encode_instruction(item.mnemonic, item.operand, labels);
        }

        return output;
    }

    static std::pair<std::string, std::string> split_mnemonic_operand(const std::string& text) {
        std::string line = trim(text);
        if (line.empty()) {
            return {"", ""};
        }

        const std::size_t pos = line.find(' ');
        if (pos == std::string::npos) {
            return {to_upper(line), ""};
        }

        std::string mnemonic = to_upper(trim(line.substr(0, pos)));
        std::string operand = trim(line.substr(pos + 1));
        return {mnemonic, operand};
    }

    static std::uint16_t encode_instruction(
        const std::string& mnemonic,
        const std::string& operand,
        const std::unordered_map<std::string, std::uint16_t>& labels) {

        const std::string upper_mnemonic = to_upper(mnemonic);
        const std::string upper_operand = to_upper(operand);

        if (upper_mnemonic == "NOP") return static_cast<std::uint16_t>(Opcode::NOP) << 8;
        if (upper_mnemonic == "LDA") return encode_load_like(Opcode::LDA, operand, labels);
        if (upper_mnemonic == "STA") return encode_store_like(Opcode::STA, operand, labels);
        if (upper_mnemonic == "MOV") return encode_mov(operand, labels);
        if (upper_mnemonic == "ADD") return encode_alu_like(Opcode::ADD, operand, labels);
        if (upper_mnemonic == "SUB") return encode_alu_like(Opcode::SUB, operand, labels);
        if (upper_mnemonic == "MUL") return encode_alu_like(Opcode::MUL, operand, labels);
        if (upper_mnemonic == "DIV") return encode_alu_like(Opcode::DIV, operand, labels);
        if (upper_mnemonic == "INC") return encode_inc_dec(Opcode::INC, operand);
        if (upper_mnemonic == "DEC") return encode_inc_dec(Opcode::DEC, operand);
        if (upper_mnemonic == "NEG") return encode_alu_like(Opcode::NEG, operand, labels);
        if (upper_mnemonic == "CMP") return encode_alu_like(Opcode::CMP, operand, labels);

        if (upper_mnemonic == "JMP") return encode_jump_like(Opcode::JMP, operand, labels);
        if (upper_mnemonic == "JZ") return encode_jump_like(Opcode::JZ, operand, labels);
        if (upper_mnemonic == "JNZ") return encode_jump_like(Opcode::JNZ, operand, labels);
        if (upper_mnemonic == "JC") return encode_jump_like(Opcode::JC, operand, labels);
        if (upper_mnemonic == "JNC") return encode_jump_like(Opcode::JNC, operand, labels);
        if (upper_mnemonic == "JS") return encode_jump_like(Opcode::JS, operand, labels);
        if (upper_mnemonic == "JNS") return encode_jump_like(Opcode::JNS, operand, labels);
        if (upper_mnemonic == "JGT") return encode_jump_like(Opcode::JGT, operand, labels);
        if (upper_mnemonic == "JGE") return encode_jump_like(Opcode::JGE, operand, labels);
        if (upper_mnemonic == "JLT") return encode_jump_like(Opcode::JLT, operand, labels);
        if (upper_mnemonic == "JLE") return encode_jump_like(Opcode::JLE, operand, labels);

        if (upper_mnemonic == "HLT") return static_cast<std::uint16_t>(Opcode::HLT) << 8;
        if (upper_mnemonic == "SHL") return encode_shift_like(Opcode::SHL, operand);
        if (upper_mnemonic == "SHR") return encode_shift_like(Opcode::SHR, operand);

        if (upper_mnemonic == "ADDL") return encode_long_arith_like(Opcode::ADDL, operand);
        if (upper_mnemonic == "SUBL") return encode_long_arith_like(Opcode::SUBL, operand);
        if (upper_mnemonic == "MULL") return encode_long_arith_like(Opcode::MULL, operand);
        if (upper_mnemonic == "CMP32") return encode_long_arith_like(Opcode::CMP32, operand);

        if (upper_mnemonic == "LDS") return encode_load_like(Opcode::LDS, operand, labels);
        if (upper_mnemonic == "STS") return encode_store_like(Opcode::STS, operand, labels);

        throw std::runtime_error("Unknown mnemonic: " + mnemonic);
    }

private:
    static std::string trim(const std::string& s) {
        std::size_t begin = 0;
        while (begin < s.size() && std::isspace(static_cast<unsigned char>(s[begin]))) {
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

    static std::uint16_t encode_load_like(
        Opcode op,
        const std::string& operand,
        const std::unordered_map<std::string, std::uint16_t>& labels) {
        std::uint16_t value = 0;

        if (operand.empty()) {
            return (static_cast<std::uint16_t>(op) << 8) | 0;
        }

        if (operand.starts_with("#")) {
            const std::string number = trim(operand.substr(1));
            value = static_cast<std::uint16_t>(std::stoi(number));
            return (static_cast<std::uint16_t>(op) << 8) | value;
        }

        if (auto it = labels.find(operand); it != labels.end()) {
            value = it->second;
            return (static_cast<std::uint16_t>(op) << 8) | value;
        }

        if (is_register_name(operand)) {
            value = register_id(operand);
            return (static_cast<std::uint16_t>(op) << 8) | value;
        }

        if (operand.starts_with("(" ) && operand.ends_with(")")) {
            std::string inner = operand.substr(1, operand.size() - 2);
            value = register_id(inner);
            return (static_cast<std::uint16_t>(op) << 8) | value;
        }

        return (static_cast<std::uint16_t>(op) << 8) | 0;
    }

    static std::uint16_t encode_store_like(
        Opcode op,
        const std::string& operand,
        const std::unordered_map<std::string, std::uint16_t>& labels) {
        if (operand.empty()) {
            return (static_cast<std::uint16_t>(op) << 8) | 0;
        }

        if (auto it = labels.find(operand); it != labels.end()) {
            return (static_cast<std::uint16_t>(op) << 8) | it->second;
        }

        if (is_register_name(operand)) {
            return (static_cast<std::uint16_t>(op) << 8) | register_id(operand);
        }

        if (operand.starts_with("(" ) && operand.ends_with(")")) {
            std::string inner = operand.substr(1, operand.size() - 2);
            return (static_cast<std::uint16_t>(op) << 8) | register_id(inner);
        }

        return (static_cast<std::uint16_t>(op) << 8) | 0;
    }

    static std::uint16_t encode_mov(const std::string& operand,
                                   const std::unordered_map<std::string, std::uint16_t>& labels) {
        std::string left, right;
        auto comma_pos = operand.find(',');
        if (comma_pos == std::string::npos) {
            return (static_cast<std::uint16_t>(Opcode::MOV) << 8) | 0;
        }

        left = trim(operand.substr(0, comma_pos));
        right = trim(operand.substr(comma_pos + 1));

        std::uint16_t dst = register_id(left);
        std::uint16_t src = 0;

        if (right.starts_with("#")) {
            src = static_cast<std::uint16_t>(std::stoi(trim(right.substr(1))));
        } else if (is_register_name(right)) {
            src = register_id(right);
        } else if (auto it = labels.find(right); it != labels.end()) {
            src = it->second;
        }

        return (static_cast<std::uint16_t>(Opcode::MOV) << 8) | (dst << 4) | src;
    }

    static std::uint16_t encode_alu_like(
        Opcode op,
        const std::string& operand,
        const std::unordered_map<std::string, std::uint16_t>& labels) {
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

        if (operand.starts_with("(" ) && operand.ends_with(")")) {
            std::string inner = operand.substr(1, operand.size() - 2);
            value = register_id(inner);
            return (static_cast<std::uint16_t>(op) << 8) | value;
        }

        return (static_cast<std::uint16_t>(op) << 8) | 0;
    }

    static std::uint16_t encode_jump_like(
        Opcode op,
        const std::string& operand,
        const std::unordered_map<std::string, std::uint16_t>& labels) {
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

    static std::uint16_t encode_long_arith_like(Opcode op, const std::string& operand) {
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
};

} // namespace cpu
