module;

#include <cstdint>
#include <limits>

export module alu;

import types;

export namespace cpu {

class ALU {
public:
    static ALUResult16 add16(std::int16_t lhs, std::int16_t rhs) noexcept {
        const std::uint32_t sum =
            static_cast<std::uint32_t>(static_cast<std::uint16_t>(lhs)) +
            static_cast<std::uint32_t>(static_cast<std::uint16_t>(rhs));

        const std::int16_t result = static_cast<std::int16_t>(sum & 0xFFFFu);
        const bool carry = sum > 0xFFFFu;
        const bool overflow =
            ((lhs < 0) == (rhs < 0)) && ((result < 0) != (lhs < 0));

        return {result, result == 0, result < 0, carry, overflow};
    }

    static ALUResult16 sub16(std::int16_t lhs, std::int16_t rhs) noexcept {
        return add16(lhs, static_cast<std::int16_t>(~rhs + 1));
    }

    static ALUResult16 mul16(std::int16_t lhs, std::int16_t rhs) noexcept {
        const std::int32_t product =
            static_cast<std::int32_t>(lhs) * static_cast<std::int32_t>(rhs);
        const std::int16_t result = static_cast<std::int16_t>(product);

        const bool overflow = product > std::numeric_limits<std::int16_t>::max() ||
                              product < std::numeric_limits<std::int16_t>::min();

        return {result, result == 0, result < 0, false, overflow};
    }

    static ALUResult32 add32(std::int32_t lhs, std::int32_t rhs) noexcept {
        const std::int64_t sum =
            static_cast<std::int64_t>(lhs) + static_cast<std::int64_t>(rhs);

        const std::int32_t result = static_cast<std::int32_t>(sum);
        const bool overflow = (sum > std::numeric_limits<std::int32_t>::max()) ||
                              (sum < std::numeric_limits<std::int32_t>::min());

        return {result, result == 0, result < 0, false, overflow};
    }

    static ALUResult32 sub32(std::int32_t lhs, std::int32_t rhs) noexcept {
        return add32(lhs, -rhs);
    }

    static LongWord mul32(std::int16_t lhs, std::int16_t rhs) noexcept {
        const std::int32_t product =
            static_cast<std::int32_t>(lhs) * static_cast<std::int32_t>(rhs);

        LongWord word{};
        word.low = static_cast<std::uint16_t>(product & 0xFFFFu);
        word.high = static_cast<std::uint16_t>((product >> 16) & 0xFFFFu);
        return word;
    }

    static std::int16_t logicalAnd(std::int16_t lhs, std::int16_t rhs) noexcept {
        return lhs & rhs;
    }

    static std::int16_t logicalOr(std::int16_t lhs, std::int16_t rhs) noexcept {
        return lhs | rhs;
    }

    static std::int16_t logicalXor(std::int16_t lhs, std::int16_t rhs) noexcept {
        return lhs ^ rhs;
    }

    static std::int16_t negate(std::int16_t value) noexcept {
        return static_cast<std::int16_t>(-static_cast<std::int32_t>(value));
    }

    static std::int16_t compare(std::int16_t lhs, std::int16_t rhs) noexcept {
        if (lhs < rhs) return -1;
        if (lhs > rhs) return 1;
        return 0;
    }

    static std::int64_t shiftLeft(std::int64_t value, std::uint8_t amount) noexcept {
        return value << amount;
    }

    static std::int64_t shiftRight(std::int64_t value, std::uint8_t amount)
        noexcept {
        return value >> amount;
    }
};

}  // namespace cpu
