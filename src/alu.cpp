#include <cstdint>
#include <emulator/alu.hpp>
#include <iostream>

namespace cpu_emulator::core {

ALUResult16 ALU::add16(std::uint16_t lhs, std::uint16_t rhs) noexcept {
    // Arithmetic is performed on the 16-bit bit patterns.
    // uint32_t gives us the carry-out from bit 15.
    const std::uint32_t sum =
        static_cast<std::uint32_t>(lhs) + static_cast<std::uint32_t>(rhs);

    const std::uint16_t result = static_cast<std::uint16_t>(sum & 0xFFFFu);

    // Unsigned carry out of bit 15.
    const bool carry = (sum & 0x10000u) != 0;

    // Signed overflow:
    // positive + positive -> negative
    // negative + negative -> positive
    const bool overflow = ((~(lhs ^ rhs) & (lhs ^ result)) & 0x8000u) != 0;

    return {
        .value = static_cast<std::int16_t>(result),
        .zero = (result == 0),
        .negative = (result & 0x8000u) != 0,
        .carry = carry,
        .overflow = overflow
    };
}

ALUResult16 ALU::sub16(std::uint16_t lhs, std::uint16_t rhs) noexcept {
    /*
     * Perform:
     *
     *     lhs - rhs
     *
     * using a wider unsigned value.
     *
     * For subtraction, CF is interpreted here as "borrow".
     * Therefore:
     *
     *     lhs < rhs -> carry/borrow = true
     */
    const std::uint32_t lhs32 = lhs;
    const std::uint32_t rhs32 = rhs;

    const std::uint16_t result =
        static_cast<std::uint16_t>((lhs32 - rhs32) & 0xFFFFu);

    const bool carry = lhs < rhs;

    /*
     * Signed overflow for:
     *
     *     lhs - rhs
     *
     * occurs when lhs and rhs have different signs and
     * the result has a different sign from lhs.
     */
    const bool overflow = ((lhs ^ rhs) & (lhs ^ result) & 0x8000u) != 0;

    return {
        .value = static_cast<std::int16_t>(result),
        .zero = (result == 0),
        .negative = (result & 0x8000u) != 0,
        .carry = carry,
        .overflow = overflow
    };
}

ALUResult32 ALU::add32(std::uint16_t lhs, std::uint16_t rhs) noexcept {
    /*
     * 16-bit + 16-bit -> 32-bit.
     *
     * IMPORTANT:
     * Do NOT sign-extend here before calculating the long result.
     *
     * The 32-bit result represents the raw 32-bit result of the
     * two 16-bit operands.
     */
    const std::uint32_t sum =
        static_cast<std::uint32_t>(lhs) + static_cast<std::uint32_t>(rhs);

    const std::uint16_t low = static_cast<std::uint16_t>(sum & 0xFFFFu);

    const std::uint16_t high = static_cast<std::uint16_t>((sum >> 16) & 0xFFFFu);

    /*
     * Unsigned carry from bit 15.
     */
    const bool carry = (sum & 0x10000u) != 0;

    /*
     * Signed overflow of the 16-bit operation.
     *
     * The result stored in the low word is interpreted as int16.
     */
    const bool overflow = ((~(lhs ^ rhs) & (lhs ^ low)) & 0x8000u) != 0;

    return {
        .value = LongWord{.low = low, .high = high},

        .zero = (low == 0),

        .negative = (low & 0x8000u) != 0,

        .carry = carry,

        .overflow = overflow
    };
}

ALUResult32 ALU::mul32(std::uint16_t lhs, std::uint16_t rhs) noexcept {
    /*
     * MUL is signed:
     *
     *     int16 * int16 -> int32
     *
     * First reinterpret the 16-bit bit patterns as signed values.
     */
    const std::int32_t signed_lhs = static_cast<std::int16_t>(lhs);

    const std::int32_t signed_rhs = static_cast<std::int16_t>(rhs);

    const std::int32_t product = signed_lhs * signed_rhs;

    /*
     * Reinterpret the signed 32-bit result as raw bits.
     */
    const std::uint32_t raw = static_cast<std::uint32_t>(product);

    const std::uint16_t low = static_cast<std::uint16_t>(raw & 0xFFFFu);

    const std::uint16_t high = static_cast<std::uint16_t>((raw >> 16) & 0xFFFFu);

    /*
     * Does the mathematical result fit into signed 16 bits?
     */
    const bool overflow = product < -32768 || product > 32767;

    return {
        .value = LongWord{.low = low, .high = high},

        .zero = (product == 0),

        .negative = (product < 0),

        /*
         * For signed MUL, don't automatically equate carry
         * and overflow unless your ISA explicitly defines CF
         * that way.
         *
         * Here we leave CF equal to overflow only if that is
         * what your assignment expects.
         */
        .carry = false,

        .overflow = overflow
    };
}

ALUResult32 ALU::ADC32(std::uint16_t lhs, std::uint16_t rhs, bool carry_in)
    noexcept {
    const std::uint32_t sum = static_cast<std::uint32_t>(lhs) +
                              static_cast<std::uint32_t>(rhs) +
                              static_cast<std::uint32_t>(carry_in);

    const std::uint16_t low = static_cast<std::uint16_t>(sum & 0xFFFFu);

    const std::uint16_t high = static_cast<std::uint16_t>((sum >> 16) & 0xFFFFu);

    const bool carry = (sum & 0x10000u) != 0;

    /*
     * Signed overflow for:
     *
     *     lhs + rhs + carry_in
     *
     * Carry-in is treated as a positive 0/1 operand.
     */
    const std::int32_t signed_lhs = static_cast<std::int16_t>(lhs);

    const std::int32_t signed_rhs = static_cast<std::int16_t>(rhs);

    const std::int32_t signed_sum =
        signed_lhs + signed_rhs + static_cast<std::int32_t>(carry_in);

    const bool overflow = signed_sum < -32768 || signed_sum > 32767;

    return {
        .value = LongWord{.low = low, .high = high},

        .zero = (low == 0),

        .negative = (low & 0x8000u) != 0,

        .carry = carry,

        .overflow = overflow
    };
}

}  // namespace cpu_emulator::core
