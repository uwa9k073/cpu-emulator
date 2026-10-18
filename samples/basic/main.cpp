#include <cstddef>
#include <cstdint>
#include <iostream>
#include <string_view>
#include <vector>

#include <emulator/assembler.hpp>
#include <emulator/cpu.hpp>
#include <emulator/emulator.hpp>
#include <emulator/instruction.hpp>
#include <emulator/memory.hpp>
#include <emulator/types.hpp>

constexpr std::array<std::string_view, 24> kArraySum = {
".program",
"        LDA     Array", // берем размер массива
"        STA     R0", // записываем в RO
"        LDA     #Array", // берем адрес первого элемента
"        ADD     #1", // смещаем на следющий
"        STA     R1", // грузим адрес в R1
"        LDA     #0", // Обнулям Акк
"        STA     R2", // Уписываем в R2 регистр суммы
"Loop:   LDA     [R1]", // Читаем по адресу в памяти
"        ADD     R2", // прибавляем текущую сумму
"        STA     R2", // грузим обратно
"        LDA     R1", // грузим адрес
"        ADD     #1", // смещаем
"        STA     R1", // грузим обратно
"        LDA     R0", // грузим счетчик
"        SUB     #1", // уменьшаем
"        STA     R0", // грузим обратно
"        JNZ     Loop", // проверяем не закончился ли
"        LDA     R2", // грузим сумму в акк
"        STA     Result", // грузим в переменную
"        HLT", // конец
".data",
"Array: DB 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10",
"Result: DB 0"
};


constexpr std::array<std::string_view, 32> kArraysConv = {
".program",
"        LDA     Array1", // берем размер массива
"        STA     R0", // записываем в RO
"        LDA     #Array1", // берем адрес первого элемента
"        ADD     #1", // смещаем на следющий
"        STA     R1", // грузим адрес в R1
"        LDA     #Array2",
"        ADD     #1",
"        STA     R2",
"        LDA     #0", // Обнулям Акк
"        STA     R3", // Уписываем в R2 регистр суммы
"Loop:   LDA     [R1]", // Читаем по адресу в памяти
"        MUL     [R2]", // прибавляем текущую сумму
"        ADC     R3",
"        STA     R3", // грузим обратно
"        LDA     R1", // грузим адрес
"        ADD     #1", // смещаем
"        STA     R1", // грузим обратно
"        LDA     R2", // грузим адрес
"        ADD     #1", // смещаем
"        STA     R2", // грузим обратно
"        LDA     R0", // грузим счетчик
"        SUB     #1", // уменьшаем
"        STA     R0", // грузим обратно
"        JNZ     Loop", // проверяем не закончился ли
"        LDA     R3", // грузим сумму в акк
"        STA     Result", // грузим в переменную
"        HLT", // конец
".data",
"Array1: DB 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10",
"Array2: DB 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10",
"Result: DW 0"
};

int main() {
    using emulator::Assembler;
    using emulator::Emulator;

    std::cout << "ASSEMBLING\n";

    Assembler assembler;
    const auto& assembled = assembler.Assemble(kArraysConv);

    std::vector<std::uint16_t> machine_code;
    machine_code.reserve(assembled.program.size());
    //
    std::cout << "SIZE: " << assembled.program.size() << '\n';

    for (const auto& line : assembled.program) {
        machine_code.push_back(line.machine_code);
        std::cout << line.address << " : "
                  << emulator::kOperationToString.TryFind(line.opcode).value() << " "
                  << static_cast<int>(line.mode) << " " << line.operand << " => 0x"
                  << std::hex << line.machine_code << std::dec << '\n';
    }

    Emulator emu(4096, 256);
    std::cout << "LOAD DATA\n";
    for (const auto& pair : assembled.data) {
        emu.load_data(assembled.data_labels.at(pair.first).first, pair.second);
    }
    std::cout << "LOAD INSTRUCTIONS\n";
    emu.load_program(machine_code);

    std::cout << "RUN\n";

    emu.run();

    std::cout << "Result = "
              << emu.read_data(assembled.data_labels.at("Result").first) << '\n';


    return 0;
}
