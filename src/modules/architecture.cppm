module;

#include <cstdint>

export module architecture;

import types;

export namespace cpu {

struct ProcessorConfig {
    std::uint16_t memory_size = 4096;
    std::uint16_t data_memory_size = 4096;
};

class Processor {
public:
    Processor() = default;
    explicit Processor(ProcessorConfig cfg) : config_(cfg) {
        reset();
    }

    void reset() noexcept {
        regs_ = {};
    }

    Registers& registers() noexcept {
        return regs_;
    }

    const Registers& registers() const noexcept {
        return regs_;
    }

    Flags& flags() noexcept {
        return regs_.flags;
    }

    const Flags& flags() const noexcept {
        return regs_.flags;
    }

    void set_register(Register reg, std::int16_t value) noexcept {
        switch (reg) {
            case Register::A: regs_.a = value; break;
            case Register::R0: regs_.r0 = value; break;
            case Register::R1: regs_.r1 = value; break;
            case Register::R2: regs_.r2 = value; break;
            case Register::R3: regs_.r3 = value; break;
            case Register::R4: regs_.r4 = value; break;
            case Register::R5: regs_.r5 = value; break;
            case Register::R6: regs_.r6 = value; break;
            case Register::R7: regs_.r7 = value; break;
            case Register::H: regs_.h = value; break;
            case Register::L: regs_.l = value; break;
            case Register::PC: regs_.pc = static_cast<std::uint16_t>(value); break;
            case Register::IR: regs_.ir = static_cast<std::uint16_t>(value); break;
            case Register::FlagsReg: break;
        }
    }

    [[nodiscard]] std::int16_t get_register(Register reg) const noexcept {
        switch (reg) {
            case Register::A: return regs_.a;
            case Register::R0: return regs_.r0;
            case Register::R1: return regs_.r1;
            case Register::R2: return regs_.r2;
            case Register::R3: return regs_.r3;
            case Register::R4: return regs_.r4;
            case Register::R5: return regs_.r5;
            case Register::R6: return regs_.r6;
            case Register::R7: return regs_.r7;
            case Register::H: return regs_.h;
            case Register::L: return regs_.l;
            case Register::PC: return static_cast<std::int16_t>(regs_.pc);
            case Register::IR: return static_cast<std::int16_t>(regs_.ir);
            case Register::FlagsReg: return 0;
        }
        return 0;
    }

    void update_flags_from_result(std::int16_t value) noexcept {
        regs_.flags.z = (value == 0);
        regs_.flags.n = (value < 0);
    }

private:
    ProcessorConfig config_{};
    Registers regs_{};
};

} // namespace cpu
