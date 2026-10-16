#pragma once

#include <cstdint>
#include <emulator/types.hpp>
namespace emulator {

class ALU {
public:
    static ALUResult16 add16(std::uint16_t lhs, std::uint16_t rhs) noexcept;

    static ALUResult16 sub16(std::int16_t lhs, std::int16_t rhs) noexcept;

    static ALUResult16 mul16(std::int16_t lhs, std::int16_t rhs) noexcept;

    static ALUResult32 add32(std::int32_t lhs, std::int32_t rhs) noexcept;

    static ALUResult32 sub32(std::int32_t lhs, std::int32_t rhs) noexcept;

    static LongWord mul32(std::int16_t lhs, std::int16_t rhs) noexcept;

    static std::int16_t logicalAnd(std::int16_t lhs, std::int16_t rhs) noexcept;

    static std::int16_t logicalOr(std::int16_t lhs, std::int16_t rhs) noexcept;

    static std::int16_t logicalXor(std::int16_t lhs, std::int16_t rhs) noexcept;

    static std::int16_t negate(std::int16_t value) noexcept;

    static std::int16_t compare(std::int16_t lhs, std::int16_t rhs) noexcept;

    static std::int64_t shiftLeft(std::int64_t value, std::uint8_t amount) noexcept;

    static std::int64_t shiftRight(std::int64_t value, std::uint8_t amount) noexcept;
};

}  // namespace emulator
