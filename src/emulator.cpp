#include <emulator/emulator.hpp>
#include <emulator/types.hpp>
#include <iostream>

namespace emulator {

Emulator::Emulator()
    : data_memory_(std::make_shared<Memory>(4096)),
      program_memory_(std::make_shared<ProgramMemory>(4096)) {
    cpu_.ConnectMemory(data_memory_, program_memory_);
}

Emulator::Emulator(std::size_t data_size, std::size_t program_size)
    : data_memory_(std::make_shared<Memory>(data_size)),
      program_memory_(std::make_shared<ProgramMemory>(program_size)) {
    cpu_.ConnectMemory(data_memory_, program_memory_);
}

void Emulator::reset() {
    data_memory_->reset();
    program_memory_->reset();
    cpu_.Reset();
}

void Emulator::load_program(std::span<MachineWord> code) { cpu_.LoadProgram(code); }

void Emulator::load_data(std::size_t address, std::span<const std::uint8_t> data) {
    if (!data_memory_) {
        throw std::runtime_error("Data memory is not initialized");
    }
    for (std::size_t i = 0; i < data.size(); ++i) {
        data_memory_->write(address + i, data[i]);
    }
}

void Emulator::step() {
    if (!cpu_.IsHalted()) {
        cpu_.ExecuteOne();
    }
    std::cout << fmt::format("{}", cpu_.GetProcessor().registers()) << '\n';
}

void Emulator::run() {
    int counter = 0;
    while (!cpu_.IsHalted()) {
        step();
        ++counter;
    }

    std::cout << "COUNT OF INSTRUCTIONS: " << counter << '\n';
}

[[nodiscard]] CPU& Emulator::cpu() noexcept { return cpu_; }

[[nodiscard]] const CPU& Emulator::cpu() const noexcept { return cpu_; }

[[nodiscard]] std::shared_ptr<Memory> Emulator::data_memory() const noexcept {
    return data_memory_;
}

[[nodiscard]] std::shared_ptr<ProgramMemory> Emulator::program_memory() const
    noexcept {
    return program_memory_;
}

[[nodiscard]] bool Emulator::halted() const noexcept { return cpu_.IsHalted(); }

[[nodiscard]] std::uint16_t Emulator::pc() const noexcept {
    return cpu_.GetProcessor().registers().pc;
}

[[nodiscard]] std::int16_t Emulator::accumulator() const noexcept {
    return cpu_.GetProcessor().get_register(Register::A);
}

[[nodiscard]] const Registers& Emulator::registers() const noexcept {
    return cpu_.GetProcessor().registers();
}

[[nodiscard]] const Flags& Emulator::flags() const noexcept {
    return cpu_.GetProcessor().flags();
}

void Emulator::write_data(std::size_t address, std::int16_t value) {
    if (!data_memory_) {
        throw std::runtime_error("Data memory is not initialized");
    }
    data_memory_->write(address, value);
}

[[nodiscard]] std::int16_t Emulator::read_data(std::size_t address) const {
    if (!data_memory_) {
        throw std::runtime_error("Data memory is not initialized");
    }
    return data_memory_->read(address);
}

void Emulator::set_register(Register reg, std::int16_t value) noexcept {
    cpu_.GetProcessor().set_register(reg, value);
}

[[nodiscard]] std::int16_t Emulator::get_register(Register reg) const noexcept {
    return cpu_.GetProcessor().get_register(reg);
}

}  // namespace emulator
