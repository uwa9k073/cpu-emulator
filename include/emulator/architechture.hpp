#pragma once
#include <cstdint>

#include <emulator/types.hpp>

namespace emulator {
class Processor {
public:
    Processor() = default;

    void reset() noexcept;

    Registers& registers() noexcept;

    const Registers& registers() const noexcept;

    Flags& flags() noexcept;

    const Flags& flags() const noexcept;

    void set_register(Register reg, std::uint16_t value) noexcept;

    [[nodiscard]] std::uint16_t get_register(Register reg) const noexcept;

    void update_flags_from_result(std::uint16_t value) noexcept;

private:
    Registers regs_{};
};

}  // namespace emulator
