#pragma once
#include <cstdint>
#include <memory>

#include <emulator/memory.hpp>
#include <emulator/types.hpp>

namespace emulator {
class Processor {
public:
    Processor() = default;

    void reset() noexcept;

    // Подключение памяти данных к процессору (для проверок границ при
    // относительной адресации).
    void connect_data_memory(std::shared_ptr<Memory> memory) noexcept;

    [[nodiscard]] std::shared_ptr<Memory> data_memory() const noexcept;

    Registers& registers() noexcept;

    const Registers& registers() const noexcept;

    Flags& flags() noexcept;

    const Flags& flags() const noexcept;

    void set_register(Register reg, std::uint16_t value) noexcept;

    [[nodiscard]] std::uint16_t get_register(Register reg) const noexcept;

    // Ширина по умолчанию для обращений к памяти, если в инструкции не задана
    // явная ширина данных.
    [[nodiscard]] static constexpr DataWidth default_data_width() noexcept {
        return DataWidth::kDW;
    }

    // Чтение из памяти данных ширины data_width по адресу address.
    // Используется для относительной (регистровой) адресации: процессор сам
    // запрашивает у подключенной памяти ее размер и проверяет, что обращение
    // всей шириной слова не выходит за границы.
    [[nodiscard]] std::uint32_t read_memory(
        std::size_t address, DataWidth data_width
    ) const;

    // Запись в память данных ширины data_width по адресу address.
    void write_memory(std::size_t address, DataWidth data_width, std::uint32_t value);

    void update_flags_from_result(std::int16_t value) noexcept;

private:
    Registers regs_{};
    std::shared_ptr<Memory> data_memory_;
};

}  // namespace emulator
