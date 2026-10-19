#pragma once
#include <cstdint>
#include <memory>
#include <optional>
#include <vector>

#include <emulator/alu.hpp>
#include <emulator/architechture.hpp>
#include <emulator/instruction.hpp>
#include <emulator/memory.hpp>
#include <emulator/types.hpp>

namespace emulator {

class CPU {
public:
    CPU() = default;

    void connect_memory(
        std::shared_ptr<Memory> data_memory,
        std::shared_ptr<ProgramMemory> program_memory
    ) noexcept;

    void reset();

    void load_program(std::span<MachineWord> machine_code);

    [[nodiscard]] std::optional<MachineWord> fetch() const;
    void execute_one();

    [[nodiscard]] bool halted() const noexcept;
    [[nodiscard]] Processor& get_processor() noexcept;

    [[nodiscard]] const Processor& get_processor() const noexcept;

    [[nodiscard]] std::shared_ptr<Memory> data_memory() const noexcept;

    [[nodiscard]] std::shared_ptr<ProgramMemory> program_memory() const noexcept;

private:
    Processor processor_{};
    std::shared_ptr<Memory> data_memory_;
    std::shared_ptr<ProgramMemory> program_memory_;
    bool halted_ = false;
};

}  // namespace emulator
