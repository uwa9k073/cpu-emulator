#include <cstdint>
#include <emulator/architechture.hpp>
#include <stdexcept>

namespace emulator {

void Processor::reset() noexcept { regs_ = {}; }

void Processor::connect_data_memory(std::shared_ptr<Memory> memory) noexcept {
    data_memory_ = std::move(memory);
}

std::shared_ptr<Memory> Processor::data_memory() const noexcept {
    return data_memory_;
}

std::uint32_t Processor::read_memory(
    std::size_t address, DataWidth data_width
) const {
    if (!data_memory_) {
        throw std::runtime_error("Data memory is not connected to processor.");
    }

    const std::size_t width = static_cast<std::size_t>(data_width);
    // Процессор запрашивает актуальный размер подключенной памяти при каждом
    // обращении, поэтому проверка границ всегда использует реальную конфигурацию.
    const std::size_t mem_size = data_memory_->size();
    if (address >= mem_size || width == 0U || address + width > mem_size) {
        throw std::out_of_range(
            "Relative (register-indirect) access out of data memory bounds"
        );
    }

    // Little-endian: младший байт лежит по меньшему адресу.
    std::uint32_t value = 0U;
    for (std::size_t i = 0; i < width; ++i) {
        value |= static_cast<std::uint32_t>(
                     static_cast<std::uint8_t>(data_memory_->read(address + i))
                 )
             << (8U * i);
    }
    return value;
}

void Processor::write_memory(
    std::size_t address, DataWidth data_width, std::uint32_t value
) {
    if (!data_memory_) {
        throw std::runtime_error("Data memory is not connected to processor.");
    }

    const std::size_t width = static_cast<std::size_t>(data_width);
    const std::size_t mem_size = data_memory_->size();
    if (address >= mem_size || width == 0U || address + width > mem_size) {
        throw std::out_of_range("Data memory write out of bounds");
    }

    // Little-endian: побайтовая запись значения шириной data_width.
    for (std::size_t i = 0; i < width; ++i) {
        data_memory_->write(
            address + i, static_cast<std::uint8_t>((value >> (8U * i)) & 0xFFU)
        );
    }
}

Registers& Processor::registers() noexcept { return regs_; }

const Registers& Processor::registers() const noexcept { return regs_; }

Flags& Processor::flags() noexcept { return regs_.flags; }

const Flags& Processor::flags() const noexcept { return regs_.flags; }

void Processor::set_register(Register reg, std::uint16_t value) noexcept {
    switch (reg) {
        case Register::A:
            regs_.a = value;
            break;
        case Register::R0:
            regs_.r0 = value;
            break;
        case Register::R1:
            regs_.r1 = value;
            break;
        case Register::R2:
            regs_.r2 = value;
            break;
        case Register::R3:
            regs_.r3 = value;
            break;
        case Register::R4:
            regs_.r4 = value;
            break;
        case Register::R5:
            regs_.r5 = value;
            break;
        case Register::R6:
            regs_.r6 = value;
            break;
        case Register::R7:
            regs_.r7 = value;
            break;
        case Register::H:
            regs_.h = value;
            break;
        case Register::L:
            regs_.l = value;
            break;
        case Register::PC:
            regs_.pc = static_cast<std::uint16_t>(value);
            break;
        case Register::IR:
            regs_.ir = static_cast<std::uint16_t>(value);
            break;
        case Register::FR:
            break;
    }
}

[[nodiscard]] std::uint16_t Processor::get_register(Register reg) const noexcept {
    switch (reg) {
        case Register::A:
            return regs_.a;
        case Register::R0:
            return regs_.r0;
        case Register::R1:
            return regs_.r1;
        case Register::R2:
            return regs_.r2;
        case Register::R3:
            return regs_.r3;
        case Register::R4:
            return regs_.r4;
        case Register::R5:
            return regs_.r5;
        case Register::R6:
            return regs_.r6;
        case Register::R7:
            return regs_.r7;
        case Register::H:
            return regs_.h;
        case Register::L:
            return regs_.l;
        case Register::PC:
            return static_cast<std::int16_t>(regs_.pc);
        case Register::IR:
            return static_cast<std::int16_t>(regs_.ir);
        case Register::FR:
            return 0;
    }
    return 0;
}

void Processor::update_flags_from_result(std::int16_t value) noexcept {
    regs_.flags.z = (value == 0);
    regs_.flags.n = (value < 0);
}

}  // namespace emulator
