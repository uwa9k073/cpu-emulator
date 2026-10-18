#pragma once

#include <cstdint>
#include <emulator/types.hpp>
namespace emulator {

class ALU {
public:
    static ALUResult16 add16(std::uint16_t lhs, std::uint16_t rhs) noexcept;

    static ALUResult16 sub16(std::uint16_t lhs, std::uint16_t rhs) noexcept;

    static ALUResult32 add32(std::uint16_t lhs, std::uint16_t rhs) noexcept;

    static ALUResult32 mul32(std::uint16_t lhs, std::uint16_t rhs) noexcept;
};

}  // namespace emulator
