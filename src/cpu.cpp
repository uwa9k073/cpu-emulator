#include <cstdint>
#include <emulator/cpu.hpp>
#include <emulator/instruction.hpp>
#include <emulator/types.hpp>

namespace emulator {

namespace {

constexpr uint16_t OPERAND_MASK = 0x03FF;       // 10 бит: 0000 0011 1111 1111
constexpr uint16_t DATA_OPERAND_MASK = 0x00FF;  // 10 бит: 0000 0011 1111 1111
constexpr uint16_t ADDR_MODE_MASK = 0x0C00;     // 2 бита:  0000 1100 0000 0000
constexpr uint16_t OPCODE_MASK = 0xF000;        // 4 бита:  1111 0000 0000 0000

constexpr int OPERAND_SHIFT = 0;
constexpr int ADDR_MODE_SHIFT = 10;
constexpr int OPCODE_SHIFT = 12;
}  // namespace

void CPU::connect_memory(
    std::shared_ptr<Memory> data_memory,
    std::shared_ptr<ProgramMemory> program_memory
) noexcept {
    data_memory_ = std::move(data_memory);
    program_memory_ = std::move(program_memory);
}

void CPU::reset() {
    processor_.reset();
    halted_ = false;
    if (program_memory_) {
        program_memory_->reset();
    }
    if (data_memory_) {
        data_memory_->reset();
    }
}

void CPU::load_program(const std::vector<std::uint16_t>& machine_code) {
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

[[nodiscard]] std::optional<std::uint16_t> CPU::fetch() const {
    if (!program_memory_) {
        return std::nullopt;
    }

    const auto pc = processor_.registers().pc;
    return program_memory_->read(pc);
}

void CPU::execute_one() {
    if (!program_memory_) {
        throw std::runtime_error("Program memory is not connected.");
    }
    if (!data_memory_) {
        throw std::runtime_error("Data memory is not connected.");
    }

    auto& regs = processor_.registers();
    const auto instruction_word = program_memory_->read(regs.pc);
    const auto opcode =
        static_cast<Opcode>((instruction_word & OPCODE_MASK) >> OPCODE_SHIFT);
    const auto mode = static_cast<AddressingMode>(
        (instruction_word & ADDR_MODE_MASK) >> ADDR_MODE_SHIFT
    );
    const auto operand_byte = static_cast<std::uint16_t>(
        (instruction_word &
         (mode == AddressingMode::Direct ? DATA_OPERAND_MASK : OPERAND_MASK)) >>
        OPERAND_SHIFT
    );

    switch (opcode) {
        case Opcode::kLDA: {
            std::uint16_t value = 0;
            if (mode == AddressingMode::Register) {
                value = processor_.get_register(static_cast<Register>(operand_byte));
            } else if (mode == AddressingMode::Immediate) {
                value = static_cast<std::uint16_t>(operand_byte);
            } else if (mode == AddressingMode::Direct) {
                value = data_memory_->read(operand_byte);
            } else if (mode == AddressingMode::IndirectRegister) {
                value = data_memory_->read(
                    processor_.get_register(static_cast<Register>(operand_byte))
                );
            }
            processor_.set_register(Register::A, value);
            processor_.update_flags_from_result(value);
            break;
        }

        case Opcode::kSTA: {
            const auto value = processor_.get_register(Register::A);
            if (mode == AddressingMode::Register) {
                processor_.set_register(static_cast<Register>(operand_byte), value);
            } else {
                data_memory_->write(operand_byte, value);
            }
            break;
        }

        case Opcode::kADD: {
            std::uint16_t value = 0;

            if (mode == AddressingMode::Register) {
                value = processor_.get_register(static_cast<Register>(operand_byte));
            } else if (mode == AddressingMode::IndirectRegister) {
                value = data_memory_->read(
                    processor_.get_register(static_cast<Register>(operand_byte))
                );
            } else if (mode == AddressingMode::Direct) {
                value = data_memory_->read(operand_byte);
            } else {
                value = operand_byte;
            }

            const auto result =
                ALU::add16(processor_.get_register(Register::A), value);
            processor_.set_register(Register::A, result.value);
            regs.flags.z = result.zero;
            regs.flags.n = result.negative;
            regs.flags.c = result.carry;
            regs.flags.o = result.overflow;
            break;
        }

        case Opcode::kSUB: {
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

        case Opcode::kMUL: {
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

        case Opcode::kJNZ: {
            if (!regs.flags.z) {
                regs.pc = operand_byte;
                return;
            }
            break;
        }

        case Opcode::kJC: {
            if (regs.flags.c) {
                regs.pc = operand_byte;
                return;
            }
            break;
        }

        case Opcode::kJNC: {
            if (!regs.flags.c) {
                regs.pc = operand_byte;
                return;
            }
            break;
        }

        case Opcode::kHLT:
            halted_ = true;
            return;
        case Opcode::kADC:
        case Opcode::kSBB:
        case Opcode::kCLC:
        case Opcode::kCMC:
        case Opcode::kSTC:
            throw std::runtime_error("This instruction is not fully implemented yet."
            );
    }

    ++regs.pc;
}

[[nodiscard]] bool CPU::halted() const noexcept { return halted_; }

[[nodiscard]] Processor& CPU::get_processor() noexcept { return processor_; }

[[nodiscard]] const Processor& CPU::get_processor() const noexcept {
    return processor_;
}

[[nodiscard]] std::shared_ptr<Memory> CPU::data_memory() const noexcept {
    return data_memory_;
}

[[nodiscard]] std::shared_ptr<ProgramMemory> CPU::program_memory() const noexcept {
    return program_memory_;
}

}  // namespace emulator
