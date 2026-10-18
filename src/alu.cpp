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
    const std::uint32_t sum =
        static_cast<std::uint32_t>(lhs) + static_cast<std::uint32_t>(rhs);

    const bool overflow = (sum > std::numeric_limits<std::uint32_t>::max()) ||
                          (sum < std::numeric_limits<std::uint32_t>::min());

    ALUResult32 result{
        .value =
            LongWord{
                .low = static_cast<uint16_t>(sum),
                .high = static_cast<uint16_t>(sum >> 16)
            },
        .zero = sum == 0,
        .negative = sum < 0,
        .carry = static_cast<uint16_t>(sum >> 16) != 0,
        .overflow = overflow
    };
    return result;
}

ALUResult32 ALU::mul32(std::uint16_t lhs, std::uint16_t rhs) noexcept {
    const std::int32_t product =
        static_cast<std::int32_t>(lhs) * static_cast<std::int32_t>(rhs);

    ALUResult32 result;
    result.value.low = static_cast<std::uint16_t>(product & 0xFFFFu);
    result.value.high = static_cast<std::uint16_t>((product >> 16) & 0xFFFFu);
    result.zero = product == 0;
    result.carry = result.value.high != 0;
    result.overflow = (product > std::numeric_limits<std::uint32_t>::max()) ||
                      (product < std::numeric_limits<std::uint32_t>::min());
    return result;
}

}  // namespace emulator
