module;

#include <array>
#include <cstdint>
#include <string_view>

export module architecture;

export namespace cpu {

enum class Register : std::uint8_t {
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
    FlagsReg
};

enum class AddressingMode : std::uint8_t {
    Immediate = 0,
    Direct,
    Register,
    IndirectRegister,
    Relative,
    Indexed
};

enum class Flag : std::uint8_t {
    Zero = 0,
    Negative,
    Carry,
    Overflow
};

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
    std::int16_t a = 0;
    std::int16_t r0 = 0;
    std::int16_t r1 = 0;
    std::int16_t r2 = 0;
    std::int16_t r3 = 0;
    std::int16_t r4 = 0;
    std::int16_t r5 = 0;
    std::int16_t r6 = 0;
    std::int16_t r7 = 0;
    std::int16_t h = 0;
    std::int16_t l = 0;
    std::uint16_t pc = 0;
    std::uint16_t ir = 0;
    Flags flags{};
};

struct LongWord {
    std::uint16_t low = 0;
    std::uint16_t high = 0;

    [[nodiscard]] std::int32_t asInt32() const noexcept {
        return static_cast<std::int32_t>((static_cast<std::uint32_t>(high) << 16) | low);
    }

    [[nodiscard]] std::uint32_t asUint32() const noexcept {
        return (static_cast<std::uint32_t>(high) << 16) | static_cast<std::uint32_t>(low);
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
    std::int32_t value = 0;
    bool zero = false;
    bool negative = false;
    bool carry = false;
    bool overflow = false;
};

class ALU {
public:
    static ALUResult16 add16(std::int16_t lhs, std::int16_t rhs) noexcept {
        const std::uint32_t sum = static_cast<std::uint32_t>(static_cast<std::uint16_t>(lhs)) +
                                 static_cast<std::uint32_t>(static_cast<std::uint16_t>(rhs));
        const std::int16_t result = static_cast<std::int16_t>(sum & 0xFFFFu);

        const bool carry = sum > 0xFFFFu;
        const bool overflow = ((lhs < 0) == (rhs < 0)) && ((result < 0) != (lhs < 0));

        return {
            result,
            result == 0,
            result < 0,
            carry,
            overflow
        };
    }

    static ALUResult16 sub16(std::int16_t lhs, std::int16_t rhs) noexcept {
        return add16(lhs, static_cast<std::int16_t>(~rhs + 1));
    }

    static ALUResult16 mul16(std::int16_t lhs, std::int16_t rhs) noexcept {
        const std::int32_t product = static_cast<std::int32_t>(lhs) * static_cast<std::int32_t>(rhs);
        const std::int16_t result = static_cast<std::int16_t>(product);

        const bool overflow = product > std::numeric_limits<std::int16_t>::max() ||
                              product < std::numeric_limits<std::int16_t>::min();

        return {
            result,
            result == 0,
            result < 0,
            false,
            overflow
        };
    }

    static ALUResult32 add32(std::int32_t lhs, std::int32_t rhs) noexcept {
        const std::int64_t sum = static_cast<std::int64_t>(lhs) + static_cast<std::int64_t>(rhs);
        const std::int32_t result = static_cast<std::int32_t>(sum);
        const bool overflow = (sum > std::numeric_limits<std::int32_t>::max()) ||
                              (sum < std::numeric_limits<std::int32_t>::min());

        return {
            result,
            result == 0,
            result < 0,
            false,
            overflow
        };
    }

    static ALUResult32 sub32(std::int32_t lhs, std::int32_t rhs) noexcept {
        return add32(lhs, -rhs);
    }

    static LongWord mul32(std::int16_t lhs, std::int16_t rhs) noexcept {
        const std::int32_t product = static_cast<std::int32_t>(lhs) * static_cast<std::int32_t>(rhs);
        LongWord word{};
        word.low = static_cast<std::uint16_t>(product & 0xFFFFu);
        word.high = static_cast<std::uint16_t>((product >> 16) & 0xFFFFu);
        return word;
    }

    static long long shiftLeft(std::int64_t value, std::uint8_t amount) noexcept {
        return value << amount;
    }

    static long long shiftRight(std::int64_t value, std::uint8_t amount) noexcept {
        return value >> amount;
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
};

constexpr std::string_view to_string(Register reg) noexcept {
    switch (reg) {
        case Register::A: return "A";
        case Register::R0: return "R0";
        case Register::R1: return "R1";
        case Register::R2: return "R2";
        case Register::R3: return "R3";
        case Register::R4: return "R4";
        case Register::R5: return "R5";
        case Register::R6: return "R6";
        case Register::R7: return "R7";
        case Register::H: return "H";
        case Register::L: return "L";
        case Register::PC: return "PC";
        case Register::IR: return "IR";
        case Register::FlagsReg: return "FLAGS";
    }
    return "?";
}

constexpr std::string_view to_string(AddressingMode mode) noexcept {
    switch (mode) {
        case AddressingMode::Immediate: return "#imm";
        case AddressingMode::Direct: return "direct";
        case AddressingMode::Register: return "reg";
        case AddressingMode::IndirectRegister: return "(reg)";
        case AddressingMode::Relative: return "rel";
        case AddressingMode::Indexed: return "indexed";
    }
    return "?";
}

} // namespace cpu
