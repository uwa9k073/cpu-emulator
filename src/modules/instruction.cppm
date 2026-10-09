module;

#include <cstdint>
#include <string_view>

export module instruction;

import types;

export namespace cpu {

enum class Opcode : std::uint8_t {
    NOP = 0,
    LDA,
    STA,
    MOV,
    ADD,
    SUB,
    MUL,
    DIV,
    INC,
    DEC,
    NEG,
    CMP,
    JMP,
    JZ,
    JNZ,
    JC,
    JNC,
    JS,
    JNS,
    JGT,
    JGE,
    JLT,
    JLE,
    HLT,
    SHL,
    SHR,
    ADDL,
    SUBL,
    MULL,
    CMP32,
    LDS,
    STS
};

struct Instruction {
    Opcode opcode = Opcode::NOP;
    AddressingMode mode = AddressingMode::Immediate;
    std::int16_t operand = 0;
    std::uint8_t reg = 0;
    std::string_view mnemonic = "NOP";
    std::string_view raw;
};

struct DecodedInstruction {
    Opcode opcode = Opcode::NOP;
    AddressingMode mode = AddressingMode::Immediate;
    std::int16_t operand = 0;
    std::string_view mnemonic = "NOP";
    std::string_view operand_text;
};

class InstructionSet {
public:
    static constexpr std::string_view to_string(Opcode op) noexcept {
        switch (op) {
            case Opcode::NOP: return "NOP";
            case Opcode::LDA: return "LDA";
            case Opcode::STA: return "STA";
            case Opcode::MOV: return "MOV";
            case Opcode::ADD: return "ADD";
            case Opcode::SUB: return "SUB";
            case Opcode::MUL: return "MUL";
            case Opcode::DIV: return "DIV";
            case Opcode::INC: return "INC";
            case Opcode::DEC: return "DEC";
            case Opcode::NEG: return "NEG";
            case Opcode::CMP: return "CMP";
            case Opcode::JMP: return "JMP";
            case Opcode::JZ: return "JZ";
            case Opcode::JNZ: return "JNZ";
            case Opcode::JC: return "JC";
            case Opcode::JNC: return "JNC";
            case Opcode::JS: return "JS";
            case Opcode::JNS: return "JNS";
            case Opcode::JGT: return "JGT";
            case Opcode::JGE: return "JGE";
            case Opcode::JLT: return "JLT";
            case Opcode::JLE: return "JLE";
            case Opcode::HLT: return "HLT";
            case Opcode::SHL: return "SHL";
            case Opcode::SHR: return "SHR";
            case Opcode::ADDL: return "ADDL";
            case Opcode::SUBL: return "SUBL";
            case Opcode::MULL: return "MULL";
            case Opcode::CMP32: return "CMP32";
            case Opcode::LDS: return "LDS";
            case Opcode::STS: return "STS";
        }
        return "NOP";
    }

    static std::uint8_t to_code(Opcode op) noexcept {
        return static_cast<std::uint8_t>(op);
    }

    static Opcode from_code(std::uint8_t value) noexcept {
        return static_cast<Opcode>(value);
    }
};

} // namespace cpu
