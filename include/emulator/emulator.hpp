#pragma once

#include <cstdint>
#include <memory>
#include <span>
#include <stdexcept>
#include <vector>

#include <emulator/cpu.hpp>
#include <emulator/memory.hpp>
#include <emulator/types.hpp>

namespace cpu_emulator::core {

class Emulator {
public:
    Emulator();

    explicit Emulator(std::size_t data_size, std::size_t program_size);

    void reset();

    void load_program(std::span<MachineWord> code);

    void load_data(std::size_t address, std::span<const std::uint8_t> data);

    void step();

    void run();

    [[nodiscard]] CPU& cpu() noexcept;

    [[nodiscard]] const CPU& cpu() const noexcept;

    [[nodiscard]] std::shared_ptr<Memory> data_memory() const noexcept;

    [[nodiscard]] std::shared_ptr<ProgramMemory> program_memory() const noexcept;
    [[nodiscard]] bool halted() const noexcept;

    [[nodiscard]] std::uint16_t pc() const noexcept;

    [[nodiscard]] std::int16_t accumulator() const noexcept;

    [[nodiscard]] const Registers& registers() const noexcept;

    [[nodiscard]] const Flags& flags() const noexcept;

    void write_data(std::size_t address, std::int16_t value);

    [[nodiscard]] std::int16_t read_data(std::size_t address) const;

    void set_register(Register reg, std::int16_t value) noexcept;

    [[nodiscard]] std::int16_t get_register(Register reg) const noexcept;

private:
    std::shared_ptr<Memory> data_memory_;
    std::shared_ptr<ProgramMemory> program_memory_;
    CPU cpu_;
};

}  // namespace cpu_emulator::core
