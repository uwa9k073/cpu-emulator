#include <emulator/architechture.hpp>
#include "emulator/types.hpp"

namespace emulator {

void Processor::reset() noexcept { regs_ = {}; }

Registers& Processor::registers() noexcept { return regs_; }

const Registers& Processor::registers() const noexcept { return regs_; }

Flags& Processor::flags() noexcept { return regs_.flags; }

const Flags& Processor::flags() const noexcept { return regs_.flags; }

void Processor::set_register(Register reg, std::uint16_t value) noexcept {
    switch (reg) {
        case Register::A:
            regs_.a = value;
            break;
        case Register::R0:
            regs_.r0 = value;
            break;
        case Register::R1:
            regs_.r1 = value;
            break;
        case Register::R2:
            regs_.r2 = value;
            break;
        case Register::R3:
            regs_.r3 = value;
            break;
        case Register::R4:
            regs_.r4 = value;
            break;
        case Register::R5:
            regs_.r5 = value;
            break;
        case Register::R6:
            regs_.r6 = value;
            break;
        case Register::R7:
            regs_.r7 = value;
            break;
        case Register::H:
            regs_.h = value;
            break;
        case Register::L:
            regs_.l = value;
            break;
        case Register::PC:
            regs_.pc = static_cast<std::uint16_t>(value);
            break;
        case Register::IR:
            regs_.ir = static_cast<std::uint16_t>(value);
            break;
        case Register::FR:
            break;
    }
}

[[nodiscard]] std::uint16_t Processor::get_register(Register reg) const noexcept {
    switch (reg) {
        case Register::A:
            return regs_.a;
        case Register::R0:
            return regs_.r0;
        case Register::R1:
            return regs_.r1;
        case Register::R2:
            return regs_.r2;
        case Register::R3:
            return regs_.r3;
        case Register::R4:
            return regs_.r4;
        case Register::R5:
            return regs_.r5;
        case Register::R6:
            return regs_.r6;
        case Register::R7:
            return regs_.r7;
        case Register::H:
            return regs_.h;
        case Register::L:
            return regs_.l;
        case Register::PC:
            return static_cast<std::int16_t>(regs_.pc);
        case Register::IR:
            return static_cast<std::int16_t>(regs_.ir);
        case Register::FR:
            return 0;
    }
    return 0;
}

void Processor::update_flags_from_result(std::uint16_t value) noexcept {
    regs_.flags.z = (value == 0);

    // Bit 15 is the sign bit of a 16-bit two's-complement value.
    regs_.flags.n = (value & 0x8000u) != 0;
}

void Processor::UpdateFlags(Flags flags) noexcept { regs_.flags = flags; }

void Processor::IncrementPc() { ++regs_.pc; }
}  // namespace emulator
