#include <cstdint>
#include <emulator/alu.hpp>
#include <limits>

namespace emulator {

ALUResult16 ALU::add16(std::uint16_t lhs, std::uint16_t rhs) noexcept {
    const std::uint32_t sum =
        static_cast<std::uint32_t>(static_cast<std::uint16_t>(lhs)) +
        static_cast<std::uint32_t>(static_cast<std::uint16_t>(rhs));

    const std::int16_t result = static_cast<std::int16_t>(sum & 0xFFFFu);
    const bool carry = sum > 0xFFFFu;
    const bool overflow = ((lhs < 0) == (rhs < 0)) && ((result < 0) != (lhs < 0));

    return {result, result == 0, result < 0, carry, overflow};
}

ALUResult16 ALU::sub16(std::int16_t lhs, std::int16_t rhs) noexcept {
    return add16(lhs, static_cast<std::int16_t>(~rhs + 1));
}

ALUResult16 ALU::mul16(std::int16_t lhs, std::int16_t rhs) noexcept {
    const std::int32_t product =
        static_cast<std::int32_t>(lhs) * static_cast<std::int32_t>(rhs);
    const std::int16_t result = static_cast<std::int16_t>(product);

    const bool overflow = product > std::numeric_limits<std::int16_t>::max() ||
                          product < std::numeric_limits<std::int16_t>::min();

    return {result, result == 0, result < 0, false, overflow};
}

ALUResult32 ALU::add32(std::int32_t lhs, std::int32_t rhs) noexcept {
    const std::int64_t sum =
        static_cast<std::int64_t>(lhs) + static_cast<std::int64_t>(rhs);

    const std::int32_t result = static_cast<std::int32_t>(sum);
    const bool overflow = (sum > std::numeric_limits<std::int32_t>::max()) ||
                          (sum < std::numeric_limits<std::int32_t>::min());

    return {result, result == 0, result < 0, false, overflow};
}

ALUResult32 ALU::sub32(std::int32_t lhs, std::int32_t rhs) noexcept {
    return add32(lhs, -rhs);
}

LongWord ALU::mul32(std::int16_t lhs, std::int16_t rhs) noexcept {
    const std::int32_t product =
        static_cast<std::int32_t>(lhs) * static_cast<std::int32_t>(rhs);

    LongWord word{};
    word.low = static_cast<std::uint16_t>(product & 0xFFFFu);
    word.high = static_cast<std::uint16_t>((product >> 16) & 0xFFFFu);
    return word;
}

std::int16_t ALU::logicalAnd(std::int16_t lhs, std::int16_t rhs) noexcept {
    return lhs & rhs;
}

std::int16_t ALU::logicalOr(std::int16_t lhs, std::int16_t rhs) noexcept {
    return lhs | rhs;
}

std::int16_t ALU::logicalXor(std::int16_t lhs, std::int16_t rhs) noexcept {
    return lhs ^ rhs;
}

std::int16_t ALU::negate(std::int16_t value) noexcept {
    return static_cast<std::int16_t>(-static_cast<std::int32_t>(value));
}

std::int16_t ALU::compare(std::int16_t lhs, std::int16_t rhs) noexcept {
    if (lhs < rhs) return -1;
    if (lhs > rhs) return 1;
    return 0;
}

std::int64_t ALU::shiftLeft(std::int64_t value, std::uint8_t amount) noexcept {
    return value << amount;
}

std::int64_t ALU::shiftRight(std::int64_t value, std::uint8_t amount) noexcept {
    return value >> amount;
}

}  // namespace emulator
