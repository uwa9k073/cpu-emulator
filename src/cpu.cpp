#include <cstddef>
#include <cstdint>
#include <emulator/alu.hpp>
#include <emulator/cpu.hpp>
#include <emulator/instruction.hpp>
#include <emulator/types.hpp>
#include <iostream>
#include <optional>
#include <stdexcept>

namespace emulator {

namespace {

constexpr uint16_t OPERAND_MASK = 0x03FF;       // 10 бит: 0000 0011 1111 1111
constexpr uint16_t DATA_OPERAND_MASK = 0x00FF;  // 8 бит: 0000 0000 1111 1111
constexpr uint16_t ADDR_MODE_MASK = 0x0C00;     // 2 бита: 0000 1100 0000 0000
constexpr uint16_t OPCODE_MASK = 0xF000;        // 4 бита: 1111 0000 0000 0000
constexpr uint16_t DATA_WIDTH_MASK = 0x0300;    // 2 бита: 0000 0011 0000 0000
constexpr int OPERAND_SHIFT = 0;
constexpr int DATA_WIDTH_SHIFT = 8;
constexpr int ADDR_MODE_SHIFT = 10;
constexpr int OPCODE_SHIFT = 12;

}  // namespace

void CPU::connect_memory(
    std::shared_ptr<Memory> data_memory,
    std::shared_ptr<ProgramMemory> program_memory
) noexcept {
    data_memory_ = std::move(data_memory);
    program_memory_ = std::move(program_memory);
    processor_.connect_data_memory(data_memory_);
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

std::uint32_t CPU::resolve_operand(std::uint16_t instruction_word) {
    const auto mode = static_cast<AddressingMode>(
        (instruction_word & ADDR_MODE_MASK) >> ADDR_MODE_SHIFT
    );

    std::optional<DataWidth> data_width;
    std::uint16_t raw_operand = 0;

    if (mode == AddressingMode::Direct || mode == AddressingMode::IndirectRegister) {
        // Ширина данных кодируется в этих режимах адресации.
        data_width = kDataWidthToMachineCode.TryFindBySecond(static_cast<std::uint16_t>(
            (instruction_word & DATA_WIDTH_MASK) >> DATA_WIDTH_SHIFT
        ));
        raw_operand = static_cast<std::uint16_t>(
            (instruction_word & DATA_OPERAND_MASK) >> OPERAND_SHIFT
        );
    } else {
        raw_operand = static_cast<std::uint16_t>(
            (instruction_word & OPERAND_MASK) >> OPERAND_SHIFT
        );
    }

    switch (mode) {
        case AddressingMode::Immediate:
            // Непосредственная адресация: операнд уже является значением.
            return raw_operand;

        case AddressingMode::Register:
            // Регистровая адресация: значение берется из регистра.
            return processor_.get_register(static_cast<Register>(raw_operand));

        case AddressingMode::Direct: {
            // Прямая адресация: читаем из памяти данных по абсолютному адресу.
            const DataWidth width =
                data_width.value_or(processor_.default_data_width());
            return processor_.read_memory(raw_operand, width);
        }

        case AddressingMode::IndirectRegister: {
            // Относительная (регистровая косвенная) адресация: базовый адрес
            // лежит в регистре. Процессор сам считывает размер подключенной
            // памяти и проверяет, что чтение всей ширины слова не выходит за
            // ее границы.
            const DataWidth width =
                data_width.value_or(processor_.default_data_width());
            const std::size_t base_address =
                processor_.get_register(static_cast<Register>(raw_operand));
            return processor_.read_memory(base_address, width);
        }
    }
    return 0;
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

    // Значение операнда, извлеченное с учетом режима адресации.
    const std::uint32_t operand_value = resolve_operand(instruction_word);
    // Знаковое расширение 16-битного значения операнда для арифметики.
    const auto value16 = static_cast<std::int16_t>(operand_value & 0xFFFFU);

    switch (opcode) {
        case Opcode::kLDA: {
            processor_.set_register(Register::A, static_cast<std::uint16_t>(operand_value));
            processor_.update_flags_from_result(value16);
            break;
        }

        case Opcode::kSTA: {
            const auto acc = processor_.get_register(Register::A);
            if (mode == AddressingMode::Register) {
                processor_.set_register(
                    static_cast<Register>(operand_value), acc
                );
            } else if (mode == AddressingMode::Immediate) {
                throw std::runtime_error("STA does not support immediate mode");
            } else {
                // Прямая и относительная запись: ширина берется из кодирования
                // инструкции (по умолчанию — DW).
                const DataWidth width =
                    kDataWidthToMachineCode.TryFindBySecond(static_cast<std::uint16_t>(
                        (instruction_word & DATA_WIDTH_MASK) >> DATA_WIDTH_SHIFT
                    )).value_or(processor_.default_data_width());
                processor_.write_memory(operand_value, width, acc);
            }
            break;
        }

        case Opcode::kADD: {
            const auto result = ALU::add16(processor_.get_register(Register::A), value16);
            processor_.set_register(Register::A, static_cast<std::uint16_t>(result.value));
            regs.flags.z = result.zero;
            regs.flags.n = result.negative;
            regs.flags.c = result.carry;
            regs.flags.o = result.overflow;
            break;
        }

        case Opcode::kSUB: {
            const auto result = ALU::sub16(processor_.get_register(Register::A), value16);
            processor_.set_register(Register::A, static_cast<std::uint16_t>(result.value));
            regs.flags.z = result.zero;
            regs.flags.n = result.negative;
            regs.flags.c = result.carry;
            regs.flags.o = result.overflow;
            break;
        }

        case Opcode::kMUL: {
            // Полное знаковое произведение int16 x int16 -> int32.
            // Младшее слово идет в A, старшее — в H (пара H:A хранит 32-битный
            // результат, как в длинной арифметике x86 AX:DX).
            const auto result = ALU::mul32(
                static_cast<std::int16_t>(processor_.get_register(Register::A)),
                value16
            );
            processor_.set_register(Register::A, result.low());
            processor_.set_register(Register::H, result.high());
            regs.flags.z = result.zero;
            regs.flags.n = result.negative;
            regs.flags.c = result.carry;
            regs.flags.o = result.overflow;
            break;
        }

        case Opcode::kADC: {
            // Длинная арифметика: 32-битное значение в паре H:A складывается с
            // sign-extended операндом и текущим флагом переноса.
            // CF используется как входной перенос, новый CF отражает перенос
            // через границу 32 бит.
            const auto result = ALU::add32(
                processor_.get_register(Register::A),
                processor_.get_register(Register::H),
                value16,
                regs.flags.c
            );
            processor_.set_register(Register::A, result.low());
            processor_.set_register(Register::H, result.high());
            regs.flags.z = result.zero;
            regs.flags.n = result.negative;
            regs.flags.c = result.carry;
            regs.flags.o = result.overflow;
            break;
        }

        case Opcode::kSBB: {
            // Вычитание с займом в длинной арифметике: сначала младшее слово
            // (A) с входным borrow = CF, затем старшее слово (H) вычитает
            // знаковое расширение операнда и заем от младшего слова.
            const auto lo = ALU::sbb16(
                processor_.get_register(Register::A),
                static_cast<std::uint16_t>(operand_value),
                regs.flags.c
            );
            const auto hi = ALU::sub16(
                processor_.get_register(Register::H),
                static_cast<std::uint16_t>(
                    static_cast<std::int32_t>(value16) >> 16  // знаковое расширение
                ),
                !lo.carry  // sbb16/sub16 инвертируют carry: заем => carry == 0
            );

            processor_.set_register(Register::A, static_cast<std::uint16_t>(lo.value));
            processor_.set_register(Register::H, static_cast<std::uint16_t>(hi.value));

            const auto combined =
                (static_cast<std::uint32_t>(static_cast<std::uint16_t>(hi.value)) << 16) |
                static_cast<std::uint16_t>(lo.value);
            regs.flags.z = combined == 0U;
            regs.flags.n = static_cast<std::int32_t>(combined) < 0;
            // Финальный CF — это флаг займа от старшего слова.
            regs.flags.c = hi.carry;
            regs.flags.o = hi.overflow;
            break;
        }

        case Opcode::kCLC:
            regs.flags.c = false;
            break;

        case Opcode::kSTC:
            regs.flags.c = true;
            break;

        case Opcode::kCMC:
            regs.flags.c = !regs.flags.c;
            break;

        case Opcode::kJNZ: {
            if (!regs.flags.z) {
                regs.pc = static_cast<std::uint16_t>(operand_value);
                return;
            }
            break;
        }

        case Opcode::kJC: {
            if (regs.flags.c) {
                regs.pc = static_cast<std::uint16_t>(operand_value);
                return;
            }
            break;
        }

        case Opcode::kJNC: {
            if (!regs.flags.c) {
                regs.pc = static_cast<std::uint16_t>(operand_value);
                return;
            }
            break;
        }

        case Opcode::kHLT:
            halted_ = true;
            return;
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
