#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <emulator/types.hpp>
#include <span>
#include <stdexcept>
#include <string>
#include <vector>

namespace emulator {

class Memory {
public:
    explicit Memory(std::size_t size = 4096);

    void reset();

    void write(std::size_t address, std::uint8_t value);

    std::uint8_t read(std::size_t address);

    std::span<const std::uint8_t> Read(std::size_t address, std::size_t size);
    void Write(std::size_t address, std::span<const std::uint8_t> data);

    std::size_t size() const noexcept;

    const std::vector<std::uint8_t>& raw() const noexcept;

private:
    std::vector<std::uint8_t> data_;
};

class ProgramMemory {
public:
    explicit ProgramMemory(std::size_t size = 4096);

    void reset();

    void write(std::size_t address, MachineWord value);

    MachineWord read(std::size_t address) const;

    std::size_t size() const noexcept;

private:
    std::vector<MachineWord> data_;
};

}  // namespace emulator
