#include <emulator/memory.hpp>
#include <ios>
#include <iostream>
#include "emulator/types.hpp"

namespace emulator {

Memory::Memory(std::size_t size) : data_(size, 0) {};

void Memory::reset() { data_.assign(data_.size(), 0); }

void Memory::write(std::size_t address, std::uint8_t value) {
    if (address >= data_.size()) {
        throw std::out_of_range("Memory write out of range");
    }
    data_[address] = value;
}

std::uint8_t Memory::read(std::size_t address) {
    if (address >= data_.size()) {
        throw std::out_of_range("Memory read out of range");
    }
    return data_[address];
}

std::span<const std::uint8_t> Memory::Read(std::size_t address, std::size_t size) {
    return std::span{data_.data() + address, size};
}
void Memory::Write(std::size_t address, std::span<const std::uint8_t> data) {
    if (address + data.size() > data_.size() || address + data.size() < address) {
        throw std::out_of_range("Memory write out of bounds");
    }

    std::cout << "Calling the write func\n" << data.size() << '\n';
    for (std::uint16_t val : data) {
        std::cout << val << '\n';
    }
    std::copy(data.begin(), data.end(), data_.begin() + address);
}

std::size_t Memory::size() const noexcept { return data_.size(); }

const std::vector<std::uint8_t>& Memory::raw() const noexcept { return data_; }

ProgramMemory::ProgramMemory(std::size_t size) : data_(size / 4, 0) {}

void ProgramMemory::reset() { data_.assign(data_.size(), 0); }

void ProgramMemory::write(std::size_t address, MachineWord value) {
    if (address >= data_.size()) {
        throw std::out_of_range("ProgramMemory write out of range");
    }

    data_[address] = value;
}

MachineWord ProgramMemory::read(std::size_t address) const {
    if (address >= data_.size()) {
        throw std::out_of_range("ProgramMemory read out of range");
    }
    return data_[address];
}

std::size_t ProgramMemory::size() const noexcept { return data_.size(); }

}  // namespace emulator
