module;

#include <cstdint>
#include <string>
#include <string_view>

export module instruction;

import types;

export namespace cpu {

enum class Opcode : std::uint8_t {
    NOP = 0,
    LDA,
    STA,
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
    std::string operand;
    std::string raw;
    Opcode opcode = Opcode::NOP;
    AddressingMode mode = AddressingMode::Immediate;
    std::uint16_t machine_code = 0;
    std::uint16_t address = 0;
};

class InstructionSet {
public:
    static constexpr std::string_view to_string(Opcode op) noexcept {
        switch (op) {
            case Opcode::NOP:
                return "NOP";
            case Opcode::LDA:
                return "LDA";
            case Opcode::STA:
                return "STA";
            case Opcode::ADD:
                return "ADD";
            case Opcode::SUB:
                return "SUB";
            case Opcode::MUL:
                return "MUL";
            case Opcode::DIV:
                return "DIV";
            case Opcode::INC:
                return "INC";
            case Opcode::DEC:
                return "DEC";
            case Opcode::NEG:
                return "NEG";
            case Opcode::CMP:
                return "CMP";
            case Opcode::JMP:
                return "JMP";
            case Opcode::JZ:
                return "JZ";
            case Opcode::JNZ:
                return "JNZ";
            case Opcode::JC:
                return "JC";
            case Opcode::JNC:
                return "JNC";
            case Opcode::JS:
                return "JS";
            case Opcode::JNS:
                return "JNS";
            case Opcode::JGT:
                return "JGT";
            case Opcode::JGE:
                return "JGE";
            case Opcode::JLT:
                return "JLT";
            case Opcode::JLE:
                return "JLE";
            case Opcode::HLT:
                return "HLT";
            case Opcode::SHL:
                return "SHL";
            case Opcode::SHR:
                return "SHR";
            case Opcode::ADDL:
                return "ADDL";
            case Opcode::SUBL:
                return "SUBL";
            case Opcode::MULL:
                return "MULL";
            case Opcode::CMP32:
                return "CMP32";
            case Opcode::LDS:
                return "LDS";
            case Opcode::STS:
                return "STS";
        }
        return "NOP";
    }

    static constexpr Opcode from_string(std::string_view operation) noexcept {
        if (operation == "NOP") return Opcode::NOP;
        if (operation == "LDA") return Opcode::LDA;
        if (operation == "STA") return Opcode::STA;
        if (operation == "ADD") return Opcode::ADD;
        if (operation == "SUB") return Opcode::SUB;
        if (operation == "MUL") return Opcode::MUL;
        if (operation == "DIV") return Opcode::DIV;
        if (operation == "INC") return Opcode::INC;
        if (operation == "DEC") return Opcode::DEC;
        if (operation == "NEG") return Opcode::NEG;
        if (operation == "CMP") return Opcode::CMP;
        if (operation == "JMP") return Opcode::JMP;
        if (operation == "JZ") return Opcode::JZ;
        if (operation == "JNZ") return Opcode::JNZ;
        if (operation == "JC") return Opcode::JC;
        if (operation == "JNC") return Opcode::JNC;
        if (operation == "JS") return Opcode::JS;
        if (operation == "JNS") return Opcode::JNS;
        if (operation == "JGT") return Opcode::JGT;
        if (operation == "JGE") return Opcode::JGE;
        if (operation == "JLT") return Opcode::JLT;
        if (operation == "JLE") return Opcode::JLE;
        if (operation == "HLT") return Opcode::HLT;
        if (operation == "SHL") return Opcode::SHL;
        if (operation == "SHR") return Opcode::SHR;
        if (operation == "ADDL") return Opcode::ADDL;
        if (operation == "SUBL") return Opcode::SUBL;
        if (operation == "MULL") return Opcode::MULL;
        if (operation == "CMP32") return Opcode::CMP32;
        if (operation == "LDS") return Opcode::LDS;
        if (operation == "STS") return Opcode::STS;

        return Opcode::NOP;
    }

    static std::uint8_t to_code(Opcode op) noexcept {
        return static_cast<std::uint8_t>(op);
    }

    static Opcode from_code(std::uint8_t value) noexcept {
        return static_cast<Opcode>(value);
    }
};

}  // namespace cpu
