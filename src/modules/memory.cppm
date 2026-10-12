module;

#include <array>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

export module memory;

import types;

export namespace cpu {

class Memory {
public:
    explicit Memory(std::size_t size = 4096) : data_(size, 0) {};

    void reset() { data_.assign(data_.size(), 0); }

    void write(std::size_t address, std::int16_t value) {
        if (address >= data_.size()) {
            throw std::out_of_range("Memory write out of range");
        }
        data_[address] = value;
    }

    std::int16_t read(std::size_t address) const {
        if (address >= data_.size()) {
            throw std::out_of_range("Memory read out of range");
        }
        return data_[address];
    }

    std::size_t size() const noexcept { return data_.size(); }

    const std::vector<std::int16_t>& raw() const noexcept { return data_; }

private:
    std::vector<std::int16_t> data_;
};

class ProgramMemory {
public:
    explicit ProgramMemory(std::size_t size = 4096) : data_(size, 0) {}

    void reset() { data_.assign(data_.size(), 0); }

    void write(std::size_t address, std::uint16_t value) {
        if (address >= data_.size()) {
            throw std::out_of_range("ProgramMemory write out of range");
        }
        data_[address] = static_cast<std::int16_t>(value);
    }

    std::uint16_t read(std::size_t address) const {
        if (address >= data_.size()) {
            throw std::out_of_range("ProgramMemory read out of range");
        }
        return static_cast<std::uint16_t>(data_[address]);
    }

    std::size_t size() const noexcept { return data_.size(); }

private:
    std::vector<std::int16_t> data_;
};

}  // namespace cpu
