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
    kMUL,  // Multiply: умножение
    kJNZ,  // Jump if Not Zero: условный переход (если не ноль)
    kHLT,  // Halt: остановка процессора

    // --- Команды для работы с флагом переноса (Carry Flag - CF) ---
    // Управление флагом
    kCLC,  // Clear Carry Flag: сбросить флаг переноса (CF = 0)
    kSTC,  // Set Carry Flag: установить флаг переноса (CF = 1)
    kCMC,  // Complement Carry Flag: инвертировать флаг переноса (CF = !CF)

    // Арифметика с флагом
    kADC,  // Add with Carry: сложение с учетом CF (A = A + operand + CF)
    kSBB,  // Subtract with Borrow: вычитание с учетом займа (A = A - operand - CF)

    // Переходы по флагу
    kJC,   // Jump if Carry: переход, если CF = 1
    kJNC,  // Jump if Not Carry: переход, если CF = 0
};
struct Instruction {
    std::string operand;
    std::string raw;
    Opcode opcode;
    AddressingMode mode = AddressingMode::Immediate;
    std::uint16_t machine_code = 0;
    std::uint16_t address = 0;
};

userver::utils::TrivialBiMap kOperationToString = [](auto selector) {
    return selector()
        .Case(Opcode::kLDA, "LDA")
        .Case(Opcode::kSTA, "STA")
        .Case(Opcode::kADD, "ADD")
        .Case(Opcode::kSUB, "SUB")
        .Case(Opcode::kMUL, "MUL")
        .Case(Opcode::kJNZ, "JNZ")
        .Case(Opcode::kJC, "JC")
        .Case(Opcode::kJNC, "JNC")
        .Case(Opcode::kHLT, "HLT")
        .Case(Opcode::kADC, "ADC")
        .Case(Opcode::kHLT, "HLT")
        .Case(Opcode::kSTC, "STC")
        .Case(Opcode::kCLC, "CLC")
        .Case(Opcode::kCMC, "CMC");
};

}  // namespace emulator
