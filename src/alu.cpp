#include <cstdint>
#include <emulator/alu.hpp>
#include <limits>

namespace emulator {

ALUResult16 ALU::add16(std::uint16_t lhs, std::uint16_t rhs) noexcept {
    const std::uint32_t sum =
        static_cast<std::uint32_t>(static_cast<std::uint16_t>(lhs)) +
        static_cast<std::uint32_t>(static_cast<std::uint16_t>(rhs));

    const std::int16_t result = static_cast<std::int16_t>(sum & 0xFFFF);
    const bool carry = sum > 0xFFFF;
    const bool overflow = ((lhs < 0) == (rhs < 0)) && ((result < 0) != (lhs < 0));

    return {result, result == 0, result < 0, carry, overflow};
}

ALUResult16 ALU::sub16(std::uint16_t lhs, std::uint16_t rhs) noexcept {
    return add16(lhs, static_cast<std::int16_t>(~rhs + 1));
}

ALUResult32 ALU::add32(std::uint16_t lhs, std::uint16_t rhs) noexcept {
    // Сначала приводим к знаковому 16-битному, затем расширяем до знакового
    // 32-битного
    const std::int32_t signed_lhs = static_cast<std::int16_t>(lhs);
    const std::int32_t signed_rhs = static_cast<std::int16_t>(rhs);

    const std::int32_t sum = signed_lhs + signed_rhs;

    ALUResult32 result{
        .value =
            LongWord{
                .low = static_cast<uint16_t>(sum & 0xFFFF),
                .high = static_cast<uint16_t>((sum >> 16) & 0xFFFF)
            },
        .zero = (sum == 0),
        .negative = (sum < 0),
        .carry = false,  // Для знаковой арифметики важнее overflow, но можно
                         // выставить по поведению процессора
        .overflow =
            (sum < -32768 || sum > 32767
            )  // Вышли за пределы 16 бит знакового числа!
    };
    return result;
}

ALUResult32 ALU::mul32(std::uint16_t lhs, std::uint16_t rhs) noexcept {
    // Корректное знаковое расширение для IMUL
    const std::int32_t signed_lhs = static_cast<std::int16_t>(lhs);
    const std::int32_t signed_rhs = static_cast<std::int16_t>(rhs);

    const std::int32_t product = signed_lhs * signed_rhs;

    ALUResult32 result;
    result.value.low = static_cast<std::uint16_t>(product & 0xFFFFu);
    result.value.high = static_cast<std::uint16_t>((product >> 16) & 0xFFFFu);
    result.zero = (product == 0);
    result.negative = (product < 0);

    // Переполнение знакового 16-битного диапазона
    result.overflow = (product < -32768 || product > 32767);
    result.carry = result.overflow;  // Дублируем для удобства переходов

    return result;
}

}  // namespace emulator
