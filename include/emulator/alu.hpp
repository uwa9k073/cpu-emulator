#pragma once

#include <cstdint>
#include <emulator/types.hpp>
namespace emulator {

class ALU {
public:
    static ALUResult16 add16(std::uint16_t lhs, std::uint16_t rhs) noexcept;

    // Сложение с учетом флага переноса/займа (CF): lhs + rhs + carry_in.
    // Используется командами ADC/SBB для длинной (многословной) арифметики.
    static ALUResult16 adc16(
        std::uint16_t lhs, std::uint16_t rhs, bool carry_in
    ) noexcept;

    static ALUResult16 sub16(std::uint16_t lhs, std::uint16_t rhs) noexcept;

    // Вычитание с учетом займа: lhs - rhs - borrow_in.
    static ALUResult16 sbb16(
        std::uint16_t lhs, std::uint16_t rhs, bool borrow_in
    ) noexcept;

    // Полное 32-битное знаковое произведение двух int16 (для MUL).
    static LongResult mul32(std::int16_t lhs, std::int16_t rhs) noexcept;

    // 32-битное сложение пары слов (low, high) + sign-extended операнд + carry_in
    // (для ADC в длинной арифметике: H:A = H:A + operand + CF).
    static LongResult
    add32(std::uint16_t low, std::uint16_t high, std::int16_t value, bool carry_in) noexcept;
};

}  // namespace emulator
