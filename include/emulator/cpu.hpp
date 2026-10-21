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

    void ConnectMemory(
        std::shared_ptr<Memory> data_memory,
        std::shared_ptr<ProgramMemory> program_memory
    ) noexcept;

    void Reset();

    void LoadProgram(std::span<MachineWord> machine_code);

    [[nodiscard]] std::optional<MachineWord> Fetch();
    void ExecuteOne();

    [[nodiscard]] bool IsHalted() const noexcept;
    [[nodiscard]] Processor& GetProcessor() noexcept;

    [[nodiscard]] const Processor& GetProcessor() const noexcept;

    [[nodiscard]] std::shared_ptr<Memory> GetDataMemory() const noexcept;

    [[nodiscard]] std::shared_ptr<ProgramMemory> GetProgramMemory() const noexcept;

private:
    Processor _processor{};
    std::shared_ptr<Memory> _data_memory;
    std::shared_ptr<ProgramMemory> _program_memory;
    bool _halted = false;
};

}  // namespace emulator
