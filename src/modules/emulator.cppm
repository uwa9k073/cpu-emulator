module;

#include <cstdint>
#include <memory>
#include <stdexcept>
#include <vector>

export module emulator;

import cpu;
import memory;
import types;

export namespace cpu {

class Emulator {
public:
    Emulator()
        : data_memory_(std::make_shared<Memory>(4096)),
          program_memory_(std::make_shared<ProgramMemory>(4096)) {
        cpu_.connect_memory(data_memory_, program_memory_);
    }

    explicit Emulator(std::size_t data_size, std::size_t program_size)
        : data_memory_(std::make_shared<Memory>(data_size)),
          program_memory_(std::make_shared<ProgramMemory>(program_size)) {
        cpu_.connect_memory(data_memory_, program_memory_);
    }

    void reset() {
        data_memory_->reset();
        program_memory_->reset();
        cpu_.reset();
    }

    void load_program(const std::vector<std::uint16_t>& code) {
        cpu_.load_program(code);
    }

    void load_data(std::size_t address, const std::vector<std::int16_t>& data) {
        if (!data_memory_) {
            throw std::runtime_error("Data memory is not initialized");
        }
        for (std::size_t i = 0; i < data.size(); ++i) {
            data_memory_->write(address + i, data[i]);
        }
    }

    void step() {
        if (!cpu_.halted()) {
            cpu_.execute_one();
        }
    }

    void run() {
        while (!cpu_.halted()) {
            step();
        }
    }

    [[nodiscard]] CPU& cpu() noexcept { return cpu_; }

    [[nodiscard]] const CPU& cpu() const noexcept { return cpu_; }

    [[nodiscard]] std::shared_ptr<Memory> data_memory() const noexcept {
        return data_memory_;
    }

    [[nodiscard]] std::shared_ptr<ProgramMemory> program_memory() const noexcept {
        return program_memory_;
    }

    [[nodiscard]] bool halted() const noexcept { return cpu_.halted(); }

    [[nodiscard]] std::uint16_t pc() const noexcept {
        return cpu_.get_processor().registers().pc;
    }

    [[nodiscard]] std::int16_t accumulator() const noexcept {
        return cpu_.get_processor().get_register(Register::A);
    }

    [[nodiscard]] const Registers& registers() const noexcept {
        return cpu_.get_processor().registers();
    }

    [[nodiscard]] const Flags& flags() const noexcept {
        return cpu_.get_processor().flags();
    }

    void write_data(std::size_t address, std::int16_t value) {
        if (!data_memory_) {
            throw std::runtime_error("Data memory is not initialized");
        }
        data_memory_->write(address, value);
    }

    [[nodiscard]] std::int16_t read_data(std::size_t address) const {
        if (!data_memory_) {
            throw std::runtime_error("Data memory is not initialized");
        }
        return data_memory_->read(address);
    }

    void set_register(Register reg, std::int16_t value) noexcept {
        cpu_.get_processor().set_register(reg, value);
    }

    [[nodiscard]] std::int16_t get_register(Register reg) const noexcept {
        return cpu_.get_processor().get_register(reg);
    }

private:
    std::shared_ptr<Memory> data_memory_;
    std::shared_ptr<ProgramMemory> program_memory_;
    CPU cpu_;
};

}  // namespace cpu
