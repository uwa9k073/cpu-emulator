#include <cstddef>
#include <cstdint>
#include <emulator/cpu.hpp>
#include <emulator/instruction.hpp>
#include <emulator/types.hpp>
#include <iostream>
#include <optional>
#include <vector>

namespace emulator {

namespace {
constexpr int DATA_WIDTH_SHIFT = 9;
constexpr int ADDR_MODE_SHIFT = 11;
constexpr int OPCODE_SHIFT = 13;
}  // namespace

void CPU::ConnectMemory(
    std::shared_ptr<Memory> data_memory,
    std::shared_ptr<ProgramMemory> program_memory
) noexcept {
    _data_memory = std::move(data_memory);
    _program_memory = std::move(program_memory);
}

void CPU::Reset() {
    _processor.reset();
    _halted = false;
    if (_program_memory) {
        _program_memory->reset();
    }
    if (_data_memory) {
        _data_memory->reset();
    }
}

void CPU::LoadProgram(std::span<MachineWord> machine_code) {
    if (!_program_memory) {
        throw std::runtime_error("Program memory is not connected.");
    }

    _program_memory->reset();
    for (std::size_t i = 0; i < machine_code.size(); ++i) {
        _program_memory->write(i, machine_code[i]);
    }

    _processor.reset();
    _processor.registers().pc = 0;
    _halted = false;
}

[[nodiscard]] std::optional<MachineWord> CPU::Fetch() {
    if (!_program_memory) {
        return std::nullopt;
    }

    const auto pc = _processor.get_register(Register::PC);
    _processor.registers().ir = _program_memory->read(pc);

    return _processor.registers().ir;
}

void CPU::ExecuteOne() {
    if (!_program_memory) {
        throw std::runtime_error("Program memory is not connected.");
    }
    if (!_data_memory) {
        throw std::runtime_error("Data memory is not connected.");
    }

    const auto instruction_word = Fetch().value();
    const auto opcode = static_cast<Opcode>(
        static_cast<std::uint16_t>(instruction_word >> OPCODE_SHIFT) & 0x000f
    );

    std::cout << "CMD: " << kOperationToString.TryFind(opcode).value() << '\n';
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

    operand_byte = static_cast<uint16_t>(instruction_word) & 0x01FF; // 0000 0001 1111 1111

    switch (opcode) {
        case Opcode::kLDA: {
            std::uint16_t value = 0;
            if (mode == AddressingMode::Register) {
                value = _processor.get_register(static_cast<Register>(operand_byte));
            } else if (mode == AddressingMode::Immediate) {
                value = static_cast<std::uint16_t>(operand_byte);
            } else if (mode == AddressingMode::Direct) {
                auto mem_view = _data_memory->Read(
                    operand_byte, static_cast<size_t>(data_width)
                );

                std::memcpy(&value, mem_view.data(), mem_view.size());
            } else if (mode == AddressingMode::IndirectRegister) {
                std::cout << "DATA WIDTH: "
                          << kDataWidthToString.TryFind(data_width).value() << '\n';
                auto mem_view = _data_memory->Read(
                    _processor.get_register(static_cast<Register>(operand_byte)),
                    static_cast<size_t>(data_width)
                );

                std::memcpy(&value, mem_view.data(), mem_view.size());
            }
            _processor.set_register(Register::A, value);
            _processor.update_flags_from_result(value);
            break;
        }

        case Opcode::kSTA: {
            auto value = _processor.get_register(Register::A);
            if (mode == AddressingMode::Register) {
                _processor.set_register(static_cast<Register>(operand_byte), value);
            } else {
                std::vector<uint8_t> res;
                res.reserve(2);

                for (std::size_t i = 0; i < std::min(
                                                static_cast<size_t>(data_width),
                                                static_cast<size_t>(sizeof(uint16_t))
                                            );
                     ++i) {
                    res.push_back(value);
                    value = value >> 8;
                }

                _data_memory->Write(operand_byte, res);
            }
            break;
        }

        case Opcode::kADD: {
            std::uint16_t value = 0;
            if (mode == AddressingMode::Register) {
                value = _processor.get_register(static_cast<Register>(operand_byte));
            } else if (mode == AddressingMode::Immediate) {
                value = static_cast<std::uint16_t>(operand_byte);
            } else if (mode == AddressingMode::Direct) {
                auto mem_view = _data_memory->Read(
                    operand_byte, static_cast<size_t>(data_width)
                );

                std::memcpy(&value, mem_view.data(), mem_view.size());
            } else if (mode == AddressingMode::IndirectRegister) {
                auto mem_view = _data_memory->Read(
                    _processor.get_register(static_cast<Register>(operand_byte)),
                    static_cast<size_t>(data_width)
                );

                std::memcpy(&value, mem_view.data(), mem_view.size());
            }

            const auto result =
                ALU::add16(_processor.get_register(Register::A), value);
            _processor.set_register(Register::A, result.value);
            _processor.UpdateFlags(
                {.z = result.zero,
                 .n = result.negative,
                 .c = result.carry,
                 .o = result.overflow}
            );
            break;
        }

        case Opcode::kSUB: {
            std::uint16_t value = 0;
            if (mode == AddressingMode::Register) {
                value = _processor.get_register(static_cast<Register>(operand_byte));
            } else if (mode == AddressingMode::Immediate) {
                value = static_cast<std::uint16_t>(operand_byte);
            } else if (mode == AddressingMode::Direct) {
                auto mem_view = _data_memory->Read(
                    operand_byte, static_cast<size_t>(data_width)
                );

                std::memcpy(&value, mem_view.data(), mem_view.size());
            } else if (mode == AddressingMode::IndirectRegister) {
                auto mem_view = _data_memory->Read(
                    _processor.get_register(static_cast<Register>(operand_byte)),
                    static_cast<size_t>(data_width)
                );

                std::memcpy(&value, mem_view.data(), mem_view.size());
            }

            const auto result =
                ALU::sub16(_processor.get_register(Register::A), value);

            _processor.set_register(Register::A, result.value);

            _processor.UpdateFlags(
                {.z = result.zero,
                 .n = result.negative,
                 .c = result.carry,
                 .o = result.overflow}
            );
            break;
        }

        case Opcode::kMUL: {
            std::uint16_t value = 0;
            if (mode == AddressingMode::Register) {
                value = _processor.get_register(static_cast<Register>(operand_byte));
            } else if (mode == AddressingMode::Immediate) {
                value = static_cast<std::uint16_t>(operand_byte);
            } else if (mode == AddressingMode::Direct) {
                auto mem_view = _data_memory->Read(
                    operand_byte, static_cast<size_t>(data_width)
                );

                std::memcpy(&value, mem_view.data(), mem_view.size());
            } else if (mode == AddressingMode::IndirectRegister) {
                auto mem_view = _data_memory->Read(
                    _processor.get_register(static_cast<Register>(operand_byte)),
                    static_cast<size_t>(data_width)
                );

                std::memcpy(&value, mem_view.data(), mem_view.size());
            }
            const auto result =
                ALU::mul32(_processor.get_register(Register::A), value);

            _processor.set_register(Register::A, result.value.low);

            /*
             * Keep the high half of the signed 32-bit product.
             */
            _processor.set_register(Register::H, result.value.high);
            _processor.UpdateFlags(
                {.z = result.zero,
                 .n = result.negative,
                 .c = result.carry,
                 .o = result.overflow}
            );
            break;
        }
        case Opcode::kADC: {
            std::uint16_t value = 0;
            if (mode == AddressingMode::Register) {
                value = _processor.get_register(static_cast<Register>(operand_byte));
            } else if (mode == AddressingMode::Immediate) {
                value = static_cast<std::uint16_t>(operand_byte);
            } else if (mode == AddressingMode::Direct) {
                auto mem_view = _data_memory->Read(
                    operand_byte, static_cast<size_t>(data_width)
                );

                std::memcpy(&value, mem_view.data(), mem_view.size());
            } else if (mode == AddressingMode::IndirectRegister) {
                auto mem_view = _data_memory->Read(
                    _processor.get_register(static_cast<Register>(operand_byte)),
                    static_cast<size_t>(data_width)
                );

                std::memcpy(&value, mem_view.data(), mem_view.size());
            }

            std::uint16_t carry_in = _processor.flags().c ? 1 : 0;

            // Вызываем add32, добавляя carry к операнду
            // (или можно сложить ACC + value + carry_in)
            const auto result =
                ALU::ADC32(_processor.get_register(Register::A), value, carry_in);

            _processor.set_register(Register::A, result.value.low);

            /*
             * H contains the high 16 bits of the 32-bit result.
             * Whether H should be modified on every ADC or only on
             * overflow depends on your ISA specification.
             */
            _processor.set_register(Register::H, result.value.high);

            _processor.UpdateFlags(
                {.z = result.zero,
                 .n = result.negative,
                 .c = result.carry,
                 .o = result.overflow}
            );
            break;
        }
        case Opcode::kJNZ: {
            if (!_processor.flags().z) {
                _processor.set_register(Register::PC, operand_byte);
                return;
            }
            break;
        }

        case Opcode::kHLT:
            _halted = true;
            return;
        default:
            throw std::runtime_error("This instruction is not fully implemented yet."
            );
    }

    _processor.IncrementPc();
}

[[nodiscard]] bool CPU::IsHalted() const noexcept { return _halted; }

[[nodiscard]] Processor& CPU::GetProcessor() noexcept { return _processor; }

[[nodiscard]] const Processor& CPU::GetProcessor() const noexcept {
    return _processor;
}

[[nodiscard]] std::shared_ptr<Memory> CPU::GetDataMemory() const noexcept {
    return _data_memory;
}

[[nodiscard]] std::shared_ptr<ProgramMemory> CPU::GetProgramMemory() const noexcept {
    return _program_memory;
}

}  // namespace emulator
