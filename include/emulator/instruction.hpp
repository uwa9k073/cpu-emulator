#pragma once

#include <cstdint>
#include <emulator/types.hpp>
#include <string>
#include <userver/utils/string_literal.hpp>
#include <userver/utils/trivial_map.hpp>

namespace emulator {
// Уникальные команды (опкоды) процессора
// Уникальные команды (опкоды) процессора
enum class Opcode : std::uint16_t {
    // --- Базовые команды (из array_sum.asm и convolution.asm) ---
    kLDA,  // Load Accumulator: загрузить в аккумулятор
    kSTA,  // Store Accumulator: сохранить аккумулятор в память/регистр
    kADD,  // Add: сложение
    kSUB,  // Subtract: вычитание
    kJNZ,  // Jump if Not Zero: условный переход (если не ноль)
    kHLT,  // Halt: остановка процессора
    // Арифметика с флагом
    kADC,  // Add with Carry: сложение с учетом CF (A = A + operand + CF)
    kMUL,  // Multiply: умножение
};
struct Instruction {
    std::string operand;
    std::string raw;
    Opcode opcode;
    AddressingMode mode = AddressingMode::Immediate;
    DataWidth width = DataWidth::kDB; // <--- ДОБАВЛЕНО: для хранения ширины данных
    MachineWord machine_code = 0;
    std::uint16_t address = 0;
};

constexpr userver::utils::TrivialBiMap kOperationToString = [](auto selector) {
    return selector()
        .Case(Opcode::kLDA, "LDA")
        .Case(Opcode::kSTA, "STA")
        .Case(Opcode::kADD, "ADD")
        .Case(Opcode::kSUB, "SUB")
        .Case(Opcode::kMUL, "MUL")
        .Case(Opcode::kJNZ, "JNZ")
        .Case(Opcode::kHLT, "HLT")
        .Case(Opcode::kADC, "ADC");
};

}  // namespace emulator
