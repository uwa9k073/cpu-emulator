#include <cstdint>
#include <emulator/alu.hpp>
#include <limits>

namespace emulator {

namespace {

// Общее 16-битное сложение двух слов с входным переносом.
// Возвращает сумму по модулю 2^16, признак выхода за границу беззнакового
// 16-битного поля (carry) и знаковое переполнение (overflow).
struct RawSum16 {
    std::uint16_t sum;
    bool carry;
    bool overflow;
};

RawSum16
add_with_carry_raw(std::uint16_t lhs, std::uint16_t rhs, bool carry_in) noexcept {
    const std::uint32_t a = lhs;
    const std::uint32_t b = rhs;
    const std::uint32_t c = carry_in ? 1U : 0U;

    const std::uint32_t partial = a + b;
    const std::uint32_t total = partial + c;

    const auto sum = static_cast<std::uint16_t>(total & 0xFFFFU);
    // Перенос возникает, если хотя бы один из двух шагов сложения вышел за 16 бит.
    const bool carry = ((partial >> 16) | (total >> 16)) != 0U;

    const auto s_lhs = static_cast<std::int16_t>(lhs);
    const auto s_rhs = static_cast<std::int16_t>(rhs);
    const auto s_sum = static_cast<std::int16_t>(sum);
    // Классическое определение overflow: знаки операндов совпадают,
    // но знак результата отличается от их знака.
    const bool overflow =
        ((s_lhs ^ s_sum) & (s_rhs ^ s_sum) & 0x8000) != 0;

    return {sum, carry, overflow};
}

ALUResult16 make_result16(RawSum16 raw) noexcept {
    const auto value = static_cast<std::int16_t>(raw.sum);
    return ALUResult16{
        .value = value,
        .zero = raw.sum == 0U,
        .negative = value < 0,
        .carry = raw.carry,
        .overflow = raw.overflow,
    };
}

}  // namespace

ALUResult16 ALU::add16(std::uint16_t lhs, std::uint16_t rhs) noexcept {
    return make_result16(add_with_carry_raw(lhs, rhs, false));
}

ALUResult16 ALU::adc16(
    std::uint16_t lhs, std::uint16_t rhs, bool carry_in
) noexcept {
    return make_result16(add_with_carry_raw(lhs, rhs, carry_in));
}

ALUResult16 ALU::sub16(std::uint16_t lhs, std::uint16_t rhs) noexcept {
    // A - B == A + ~B + 1. Единичный перенос из этого выражения инвертируется
    // в флаг займа (carry == 0 означает, что заем был).
    const auto raw = add_with_carry_raw(lhs, static_cast<std::uint16_t>(~rhs), true);
    ALUResult16 result = make_result16(raw);
    result.carry = !raw.carry;
    return result;
}

ALUResult16 ALU::sbb16(
    std::uint16_t lhs, std::uint16_t rhs, bool borrow_in
) noexcept {
    // A - B - borrow == A + ~B + (1 - borrow).
    const auto raw =
        add_with_carry_raw(lhs, static_cast<std::uint16_t>(~rhs), !borrow_in);
    ALUResult16 result = make_result16(raw);
    result.carry = !raw.carry;
    return result;
}

LongResult ALU::mul32(std::int16_t lhs, std::int16_t rhs) noexcept {
    const std::int32_t product =
        static_cast<std::int32_t>(lhs) * static_cast<std::int32_t>(rhs);

    const auto unsigned_product = static_cast<std::uint32_t>(product);
    const auto high = static_cast<std::uint16_t>(product >> 16);
    const auto low = static_cast<std::uint16_t>(product & 0xFFFFU);

    return LongResult{
        .value = unsigned_product,
        .zero = product == 0,
        .negative = product < 0,
        // Перенос для "короткого" MUL: старшее слово ненулевое, т.е. результат
        // не помещается в 16 бит.
        .carry = high != 0U,
        // Знаковое переполнение: результат не представим в int16.
        .overflow = product > std::numeric_limits<std::int16_t>::max() ||
                    product < std::numeric_limits<std::int16_t>::min(),
    };
}

LongResult ALU::add32(
    std::uint16_t low, std::uint16_t high, std::int16_t value, bool carry_in
) noexcept {
    // Операнд расширяется знаком до 32 бит (sign-extension).
    const auto extended = static_cast<std::uint32_t>(static_cast<std::int32_t>(value));
    const std::uint32_t current =
        (static_cast<std::uint32_t>(high) << 16) | low;

    const std::uint32_t total = current + extended + (carry_in ? 1U : 0U);

    // Беззнаковый перенос через границу 32 бит.
    const bool carry = total < current || (carry_in && total == current);

    // Знаковое переполнение 32-битного результата.
    const auto s_a = static_cast<std::int32_t>(current);
    const auto s_b = static_cast<std::int32_t>(extended);
    const auto s_sum = static_cast<std::int32_t>(total);
    const bool overflow = ((s_a ^ s_sum) & (s_b ^ s_sum)) < 0;

    return LongResult{
        .value = total,
        .zero = total == 0U,
        .negative = s_sum < 0,
        .carry = carry,
        .overflow = overflow,
    };
}

}  // namespace emulator
