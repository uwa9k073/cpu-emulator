module;

#include <cstdint>
#include <memory>
#include <optional>
#include <stdexcept>
#include <vector>

export module cpu;

import alu;
import architecture;
import instruction;
import memory;
import types;

export namespace cpu {

class CPU {
public:
    CPU() = default;

    void connect_memory(
        std::shared_ptr<Memory> data_memory,
        std::shared_ptr<ProgramMemory> program_memory
    ) noexcept {
        data_memory_ = std::move(data_memory);
        program_memory_ = std::move(program_memory);
    }

    void reset() {
        processor_.reset();
        halted_ = false;
        if (program_memory_) {
            program_memory_->reset();
        }
        if (data_memory_) {
            data_memory_->reset();
        }
    }

    void load_program(const std::vector<std::uint16_t>& machine_code) {
        if (!program_memory_) {
            throw std::runtime_error("Program memory is not connected.");
        }

        program_memory_->reset();
        for (std::size_t i = 0; i < machine_code.size(); ++i) {
            program_memory_->write(i, machine_code[i]);
        }

        processor_.reset();
        processor_.registers().pc = 0;
        halted_ = false;
    }

    [[nodiscard]] std::optional<std::uint16_t> fetch() const {
        if (!program_memory_) {
            return std::nullopt;
        }

        const auto pc = processor_.registers().pc;
        return program_memory_->read(pc);
    }

    void execute_one() {
        if (!program_memory_) {
            throw std::runtime_error("Program memory is not connected.");
        }
        if (!data_memory_) {
            throw std::runtime_error("Data memory is not connected.");
        }

        auto& regs = processor_.registers();
        const auto instruction_word = program_memory_->read(regs.pc);
        const auto opcode = static_cast<Opcode>(instruction_word >> 8);
        const auto operand_byte =
            static_cast<std::uint8_t>(instruction_word & 0xFFu);

        switch (opcode) {
            case Opcode::NOP:
                break;

            case Opcode::LDA: {
                const auto value = static_cast<std::int16_t>(operand_byte);
                processor_.set_register(Register::A, value);
                processor_.update_flags_from_result(value);
                break;
            }

            case Opcode::STA: {
                const auto value = processor_.get_register(Register::A);
                data_memory_->write(operand_byte, value);
                break;
            }

            case Opcode::ADD: {
                const auto value = static_cast<std::int16_t>(operand_byte);
                const auto result =
                    ALU::add16(processor_.get_register(Register::A), value);
                processor_.set_register(Register::A, result.value);
                regs.flags.z = result.zero;
                regs.flags.n = result.negative;
                regs.flags.c = result.carry;
                regs.flags.o = result.overflow;
                break;
            }

            case Opcode::SUB: {
                const auto value = static_cast<std::int16_t>(operand_byte);
                const auto result =
                    ALU::sub16(processor_.get_register(Register::A), value);
                processor_.set_register(Register::A, result.value);
                regs.flags.z = result.zero;
                regs.flags.n = result.negative;
                regs.flags.c = result.carry;
                regs.flags.o = result.overflow;
                break;
            }

            case Opcode::MUL: {
                const auto value = static_cast<std::int16_t>(operand_byte);
                const auto result =
                    ALU::mul16(processor_.get_register(Register::A), value);
                processor_.set_register(Register::A, result.value);
                regs.flags.z = result.zero;
                regs.flags.n = result.negative;
                regs.flags.c = result.carry;
                regs.flags.o = result.overflow;
                break;
            }

            case Opcode::DIV: {
                const auto divisor = static_cast<std::int16_t>(operand_byte);
                if (divisor == 0) {
                    throw std::runtime_error("Division by zero");
                }
                const auto result = processor_.get_register(Register::A) / divisor;
                processor_.set_register(Register::A, result);
                processor_.update_flags_from_result(result);
                break;
            }

            case Opcode::INC: {
                const auto reg = static_cast<Register>(operand_byte);
                auto value = processor_.get_register(reg);
                ++value;
                processor_.set_register(reg, value);
                processor_.update_flags_from_result(value);
                break;
            }

            case Opcode::DEC: {
                const auto reg = static_cast<Register>(operand_byte);
                auto value = processor_.get_register(reg);
                --value;
                processor_.set_register(reg, value);
                processor_.update_flags_from_result(value);
                break;
            }

            case Opcode::NEG: {
                auto value = static_cast<std::int16_t>(operand_byte);
                value = ALU::negate(value);
                processor_.set_register(Register::A, value);
                processor_.update_flags_from_result(value);
                break;
            }

            case Opcode::CMP: {
                const auto value = static_cast<std::int16_t>(operand_byte);
                const auto result =
                    ALU::compare(processor_.get_register(Register::A), value);
                regs.flags.z = (result == 0);
                regs.flags.n = (result < 0);
                break;
            }

            case Opcode::JMP: {
                regs.pc = operand_byte;
                return;
            }

            case Opcode::JZ: {
                if (regs.flags.z) {
                    regs.pc = operand_byte;
                    return;
                }
                break;
            }

            case Opcode::JNZ: {
                if (!regs.flags.z) {
                    regs.pc = operand_byte;
                    return;
                }
                break;
            }

            case Opcode::JC: {
                if (regs.flags.c) {
                    regs.pc = operand_byte;
                    return;
                }
                break;
            }

            case Opcode::JNC: {
                if (!regs.flags.c) {
                    regs.pc = operand_byte;
                    return;
                }
                break;
            }

            case Opcode::JS: {
                if (regs.flags.n) {
                    regs.pc = operand_byte;
                    return;
                }
                break;
            }

            case Opcode::JNS: {
                if (!regs.flags.n) {
                    regs.pc = operand_byte;
                    return;
                }
                break;
            }

            case Opcode::JGT: {
                if (!regs.flags.z && !regs.flags.n) {
                    regs.pc = operand_byte;
                    return;
                }
                break;
            }

            case Opcode::JGE: {
                if (!regs.flags.n) {
                    regs.pc = operand_byte;
                    return;
                }
                break;
            }

            case Opcode::JLT: {
                if (regs.flags.n) {
                    regs.pc = operand_byte;
                    return;
                }
                break;
            }

            case Opcode::JLE: {
                if (regs.flags.n || regs.flags.z) {
                    regs.pc = operand_byte;
                    return;
                }
                break;
            }

            case Opcode::HLT:
                halted_ = true;
                return;

            case Opcode::SHL: {
                const auto shifted = ALU::shiftLeft(
                    processor_.get_register(Register::A), operand_byte
                );
                processor_.set_register(
                    Register::A, static_cast<std::int16_t>(shifted)
                );
                break;
            }

            case Opcode::SHR: {
                const auto shifted = ALU::shiftRight(
                    processor_.get_register(Register::A), operand_byte
                );
                processor_.set_register(
                    Register::A, static_cast<std::int16_t>(shifted)
                );
                break;
            }

            case Opcode::ADDL:
            case Opcode::SUBL:
            case Opcode::MULL:
            case Opcode::CMP32:
            case Opcode::LDS:
            case Opcode::STS:
                throw std::runtime_error(
                    "This instruction is not fully implemented yet."
                );
        }

        ++regs.pc;
    }

    [[nodiscard]] bool halted() const noexcept { return halted_; }

    [[nodiscard]] Processor& get_processor() noexcept { return processor_; }

    [[nodiscard]] const Processor& get_processor() const noexcept {
        return processor_;
    }

    [[nodiscard]] std::shared_ptr<Memory> data_memory() const noexcept {
        return data_memory_;
    }

    [[nodiscard]] std::shared_ptr<ProgramMemory> program_memory() const noexcept {
        return program_memory_;
    }

private:
    Processor processor_{};
    std::shared_ptr<Memory> data_memory_;
    std::shared_ptr<ProgramMemory> program_memory_;
    bool halted_ = false;
};

}  // namespace cpu
