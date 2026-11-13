#pragma once

#include <bitset>
#include <cstdint>
#include <string_view>

#include <userver/utils/string_literal.hpp>
#include <userver/utils/trivial_map.hpp>

namespace cpu_emulator::core {

using MachineWord = std::uint16_t;

enum class Register : std::uint16_t {
    A = 0,
    R0,
    R1,
    R2,
    R3,
    R4,
    R5,
    R6,
    R7,
    H,
    L,
    PC,
    IR,
    FR
};

enum class AddressingMode : std::uint8_t {
    Immediate = 0b00,
    Direct = 0b01,
    Register = 0b10,
    IndirectRegister = 0b11
};

enum class DataWidth {
    kDB = 1,  // 1 byte
    kDW = 2,  // 2 byte
    kDD = 4,  // 4 byte
    kDQ = 8,  // 8 byte
};

constexpr userver::utils::TrivialBiMap kDataWidthToString = [](auto selector) {
    return selector()
        .Case(DataWidth::kDB, "DB")
        .Case(DataWidth::kDW, "DW")
        .Case(DataWidth::kDD, "DD")
        .Case(DataWidth::kDQ, "DQ");
};

constexpr userver::utils::TrivialBiMap kDataWidthToMachineCode = [](auto selector) {
    return selector()
        .Case(DataWidth::kDB, 0)
        .Case(DataWidth::kDW, 1)
        .Case(DataWidth::kDD, 2)
        .Case(DataWidth::kDQ, 3);
};

enum class Flag : std::uint8_t { Zero = 0, Negative, Carry, Overflow };

struct Flags {
    bool z = false;
    bool n = false;
    bool c = false;
    bool o = false;

    void clear() noexcept {
        z = false;
        n = false;
        c = false;
        o = false;
    }
};

struct Registers {
    std::uint16_t a = 0;
    std::uint16_t r0 = 0;
    std::uint16_t r1 = 0;
    std::uint16_t r2 = 0;
    std::uint16_t r3 = 0;
    std::uint16_t r4 = 0;
    std::uint16_t r5 = 0;
    std::uint16_t r6 = 0;
    std::uint16_t r7 = 0;
    std::uint16_t h = 0;
    std::uint16_t l = 0;
    std::uint16_t pc = 0;
    MachineWord ir = 0;
    Flags flags{};
};

struct LongWord {
    std::uint16_t low = 0;
    std::uint16_t high = 0;

    [[nodiscard]] std::int32_t asInt32() const noexcept {
        return static_cast<std::int32_t>(
            (static_cast<std::uint32_t>(high) << 16) | low
        );
    }

    [[nodiscard]] std::uint32_t asUint32() const noexcept {
        return (static_cast<std::uint32_t>(high) << 16) |
               static_cast<std::uint32_t>(low);
    }
};

struct ALUResult16 {
    std::int16_t value = 0;
    bool zero = false;
    bool negative = false;
    bool carry = false;
    bool overflow = false;
};

struct ALUResult32 {
    LongWord value{.low = 0, .high = 0};
    bool zero = false;
    bool negative = false;
    bool carry = false;
    bool overflow = false;
};

constexpr userver::utils::TrivialBiMap kRegisterNameString = [](auto selector) {
    return selector()
        .Case(Register::A, "A")
        .Case(Register::R0, "R0")
        .Case(Register::R1, "R1")
        .Case(Register::R2, "R2")
        .Case(Register::R3, "R3")
        .Case(Register::R4, "R4")
        .Case(Register::R5, "R5")
        .Case(Register::R6, "R6")
        .Case(Register::R7, "R7")
        .Case(Register::H, "H")
        .Case(Register::L, "L")
        .Case(Register::PC, "PC")
        .Case(Register::IR, "IR")
        .Case(Register::FR, "FR");
};

constexpr std::string_view to_string(Register reg) noexcept {
    return kRegisterNameString.TryFind(reg).value_or(
        userver::utils::StringLiteral{"?"}
    );
}
}  // namespace cpu_emulator::core

template <>
struct fmt::formatter<cpu_emulator::core::Registers> {
    // Default to hex ('x'), allow decimal ('d')
    char presentation = 'd';

    constexpr auto parse(fmt::format_parse_context& ctx) -> decltype(ctx.begin()) {
        auto it = ctx.begin(), end = ctx.end();
        if (it != end && (*it == 'x' || *it == 'd')) {
            presentation = *it++;
        }
        if (it != end && *it != '}') {
            throw fmt::format_error("invalid format");
        }
        return it;
    }

    template <typename FormatContext>
    auto format(const cpu_emulator::core::Registers& r, FormatContext& ctx) const
        -> decltype(ctx.out()) {
        if (presentation == 'x') {
            return fmt::format_to(
                ctx.out(),
                "Registers {{\n"
                "  {:<4} = 0x{:04X}   {:<4} = 0x{:04X}   {:<4} = 0x{:04X}\n"
                "  {:<4} = 0x{:04X}   {:<4} = 0x{:04X}   {:<4} = 0x{:04X}   {:<4} = "
                "0x{:04X}\n"
                "  {:<4} = 0x{:04X}   {:<4} = 0x{:04X}   {:<4} = 0x{:04X}   {:<4} = "
                "0x{:04X}\n"
                "  {:<4} = 0x{:04X}   {:<4} = 0x{:04X}\n"
                "}}",
                "a",
                r.a,
                "pc",
                r.pc,
                "ir",
                r.ir,
                "r0",
                r.r0,
                "r1",
                r.r1,
                "r2",
                r.r2,
                "r3",
                r.r3,
                "r4",
                r.r4,
                "r5",
                r.r5,
                "r6",
                r.r6,
                "r7",
                r.r7,
                "h",
                r.h,
                "l",
                r.l
            );
        } else {
            return fmt::format_to(
                ctx.out(),
                "Registers {{\n"
                "  {:<4} = {:<5}   {:<4} = {:<5}   {:<4} = {:<5}\n"
                "  {:<4} = {:<5}   {:<4} = {:<5}   {:<4} = {:<5}   {:<4} = {:<5}\n"
                "  {:<4} = {:<5}   {:<4} = {:<5}   {:<4} = {:<5}   {:<4} = {:<5}\n"
                "  {:<4} = {:<5}   {:<4} = {:<5}\n"
                "}}",
                "a",
                r.a,
                "pc",
                r.pc,
                "ir",
                r.ir,
                "r0",
                r.r0,
                "r1",
                r.r1,
                "r2",
                r.r2,
                "r3",
                r.r3,
                "r4",
                r.r4,
                "r5",
                r.r5,
                "r6",
                r.r6,
                "r7",
                r.r7,
                "h",
                r.h,
                "l",
                r.l
            );
        }
    }
};
