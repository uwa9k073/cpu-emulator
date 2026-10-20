#include <cstddef>
#include <cstdint>
#include <emulator/cpu.hpp>
#include <emulator/instruction.hpp>
#include <emulator/types.hpp>
#include <ios>
#include <iostream>
#include <iterator>
#include <optional>
#include <vector>

namespace emulator {

namespace {

constexpr uint16_t OPERAND_MASK = 0x03FF;        // 10 бит: 0000 0011 1111 1111
constexpr uint16_t DATA_OPERAND_MASK = 0x00FF;   // 10 бит: 0000 0000 1111 1111
constexpr uint16_t ADDR_MODE_MASK = 0x0C00;      // 2 бита: 0000 1100 0000 0000
constexpr MachineWord OPCODE_MASK = 0xF0000000;  // 4 бита: 1111 0000 0000 0000
constexpr uint16_t DATA_WIDTH_MASK = 0x0300;     // 2 бита: 0000 0011 0000 0000
constexpr int OPERAND_SHIFT = 0;
constexpr int DATA_WIDTH_SHIFT = 24;
constexpr int ADDR_MODE_SHIFT = 26;
constexpr int OPCODE_SHIFT = 28;
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

void CPU::load_program(std::span<MachineWord> machine_code) {
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

[[nodiscard]] std::optional<MachineWord> CPU::fetch() const {
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
    const auto opcode = static_cast<Opcode>(
        static_cast<std::uint16_t>(instruction_word >> OPCODE_SHIFT) & 0x000f
    );

    std::cout << "CMD: " << std::hex << static_cast<uint16_t>(opcode) << std::dec
              << '\n';
    const auto mode = static_cast<AddressingMode>(
        static_cast<std::uint8_t>(instruction_word >> ADDR_MODE_SHIFT) & 0x0003
    );

    std::uint16_t operand_byte;
    DataWidth data_width =
        kDataWidthToMachineCode
            .TryFindBySecond(
                static_cast<std::uint16_t>(instruction_word >> DATA_WIDTH_SHIFT) &
                0x0003
            )
            .value_or(DataWidth::kDB);

    operand_byte = static_cast<uint16_t>(instruction_word);

    switch (opcode) {
        case Opcode::kLDA: {
            std::uint16_t value = 0;
            if (mode == AddressingMode::Register) {
                value = processor_.get_register(static_cast<Register>(operand_byte));
            } else if (mode == AddressingMode::Immediate) {
                value = static_cast<std::uint16_t>(operand_byte);
            } else if (mode == AddressingMode::Direct) {
                auto mem_view = data_memory_->Read(
                    operand_byte, static_cast<size_t>(data_width)
                );

                std::memcpy(&value, mem_view.data(), mem_view.size());
            } else if (mode == AddressingMode::IndirectRegister) {
                auto mem_view = data_memory_->Read(
                    processor_.get_register(static_cast<Register>(operand_byte)),
                    static_cast<size_t>(data_width)
                );

                std::memcpy(&value, mem_view.data(), mem_view.size());
            }
            processor_.set_register(Register::A, value);
            processor_.update_flags_from_result(value);
            break;
        }

        case Opcode::kSTA: {
            auto value = processor_.get_register(Register::A);
            std::cout << "REG A VALUE: " << value << '\n';
            if (mode == AddressingMode::Register) {
                processor_.set_register(static_cast<Register>(operand_byte), value);
            } else {
                std::vector<uint8_t> res;
                res.reserve(2);

                std::cout << static_cast<size_t>(data_width);

                for (std::size_t i = 0; i < std::min(
                                                static_cast<size_t>(data_width),
                                                static_cast<size_t>(sizeof(uint16_t))
                                            );
                     ++i) {
                    res.push_back(value);
                    value = value >> 8;
                }

                std::cout << res.size() << '\n';

                data_memory_->Write(operand_byte, res);
            }
            break;
        }

        case Opcode::kADD: {
            std::uint16_t value = 0;
            if (mode == AddressingMode::Register) {
                value = processor_.get_register(static_cast<Register>(operand_byte));
            } else if (mode == AddressingMode::Immediate) {
                value = static_cast<std::uint16_t>(operand_byte);
            } else if (mode == AddressingMode::Direct) {
                auto mem_view = data_memory_->Read(
                    operand_byte, static_cast<size_t>(data_width)
                );

                std::memcpy(&value, mem_view.data(), mem_view.size());
            } else if (mode == AddressingMode::IndirectRegister) {
                auto mem_view = data_memory_->Read(
                    processor_.get_register(static_cast<Register>(operand_byte)),
                    static_cast<size_t>(data_width)
                );

                std::memcpy(&value, mem_view.data(), mem_view.size());
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
            std::uint16_t value = 0;
            if (mode == AddressingMode::Register) {
                value = processor_.get_register(static_cast<Register>(operand_byte));
            } else if (mode == AddressingMode::Immediate) {
                value = static_cast<std::uint16_t>(operand_byte);
            } else if (mode == AddressingMode::Direct) {
                auto mem_view = data_memory_->Read(
                    operand_byte, static_cast<size_t>(data_width)
                );

                std::memcpy(&value, mem_view.data(), mem_view.size());
            } else if (mode == AddressingMode::IndirectRegister) {
                auto mem_view = data_memory_->Read(
                    processor_.get_register(static_cast<Register>(operand_byte)),
                    static_cast<size_t>(data_width)
                );

                std::memcpy(&value, mem_view.data(), mem_view.size());
            }

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
            std::uint16_t value = 0;
            if (mode == AddressingMode::Register) {
                value = processor_.get_register(static_cast<Register>(operand_byte));
            } else if (mode == AddressingMode::Immediate) {
                value = static_cast<std::uint16_t>(operand_byte);
            } else if (mode == AddressingMode::Direct) {
                auto mem_view = data_memory_->Read(
                    operand_byte, static_cast<size_t>(data_width)
                );

                std::memcpy(&value, mem_view.data(), mem_view.size());
            } else if (mode == AddressingMode::IndirectRegister) {
                auto mem_view = data_memory_->Read(
                    processor_.get_register(static_cast<Register>(operand_byte)),
                    static_cast<size_t>(data_width)
                );

                std::memcpy(&value, mem_view.data(), mem_view.size());
            }
            const auto result =
                ALU::mul32(processor_.get_register(Register::A), value);
            processor_.set_register(Register::A, result.value.low);
            if (result.overflow) {
                processor_.set_register(Register::R4, result.value.high);
            }
            regs.flags.z = result.zero;
            regs.flags.n = result.negative;
            regs.flags.c = result.carry;
            regs.flags.o = result.overflow;
            break;
        }
        case Opcode::kADC: {
            std::uint16_t value = 0;
            if (mode == AddressingMode::Register) {
                value = processor_.get_register(static_cast<Register>(operand_byte));
            } else if (mode == AddressingMode::Immediate) {
                value = static_cast<std::uint16_t>(operand_byte);
            } else if (mode == AddressingMode::Direct) {
                auto mem_view = data_memory_->Read(
                    operand_byte, static_cast<size_t>(data_width)
                );

                std::memcpy(&value, mem_view.data(), mem_view.size());
            } else if (mode == AddressingMode::IndirectRegister) {
                auto mem_view = data_memory_->Read(
                    processor_.get_register(static_cast<Register>(operand_byte)),
                    static_cast<size_t>(data_width)
                );

                std::memcpy(&value, mem_view.data(), mem_view.size());
            }

            std::uint16_t carry_in = regs.flags.c ? 1 : 0;

            // Вызываем add32, добавляя carry к операнду
            // (или можно сложить ACC + value + carry_in)
            const auto result = ALU::add32(
                processor_.get_register(Register::A),
                static_cast<std::uint16_t>(value + carry_in)
            );

            processor_.set_register(Register::A, result.value.low);

            // Теперь старшая часть запишется в R4 ВСЕГДА, когда результат операции
            // (умножения/сложения) или сам перенос заставили число выйти за рамки 16
            // бит.
            if (result.overflow || result.value.high != 0) {
                processor_.set_register(Register::R4, result.value.high);
            } else {
                processor_.set_register(
                    Register::R4, 0
                );  // Полезно обнулять, если переполнения нет
            }

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

        case Opcode::kHLT:
            halted_ = true;
            return;
        default:
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
