#include <fmt/core.h>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <string_view>
#include <vector>

#include <emulator/assembler.hpp>
#include <emulator/cpu.hpp>
#include <emulator/emulator.hpp>
#include <emulator/instruction.hpp>
#include <emulator/memory.hpp>
#include <emulator/types.hpp>

constexpr std::array<std::string_view, 23> kArraySum = {
".program",
"        LDA     Array", // берем размер массива
"        STA     R0", // записываем в RO
"        LDA     #Array+1", // берем адрес первого элемента
"        STA     R1", // грузим адрес в R1
"        LDA     #0", // Обнулям Акк
"        STA     R2", // Уписываем в R2 регистр суммы
"Loop:   LDA     [R1]", // Читаем по адресу в памяти (we can additionaly describe how much bytes we can read)
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


constexpr std::array<std::string_view, 41> kArraysConv = {
    ".program",
    // R0 = количество элементов
    "        LDA     Array1",
    "        STA     R0",
    // R1 = адрес первого элемента Array1
    "        LDA     #Array1+2",
    "        STA     R1",
    // R2 = адрес первого элемента Array2
    "        LDA     #Array2+2",
    "        STA     R2",
    // R4:R3 = 32-битная сумма
    // R3 = low, R4 = high
    "        LDA     #0",
    "        STA     R3",
    "        STA     R4",
"Loop:",
    // A = Array1[i]
    "        LDA     DW [R1]",
    "        STA     R5",
    // A = Array2[i]
    "        LDA     DW [R2]",
    // H:A = Array2[i] * Array1[i]
    "        MUL     R5",
    // Сохраняем младшую часть произведения
    "        STA     R6",
    // R3 = R3 + product.low
    "        LDA     R3",
    "        ADD     R6",
    "        STA     R3",
    // R4 = R4 + product.high + carry
    "        LDA     R4",
    "        ADC     H",
    "        STA     R4",
    // R1 += 2
    "        LDA     R1",
    "        ADD     #2",
    "        STA     R1",
    // R2 += 2
    "        LDA     R2",
    "        ADD     #2",
    "        STA     R2",
    // R0--
    "        LDA     R0",
    "        SUB     #1",
    "        STA     R0",
    // если R0 != 0, повторяем
    "        JNZ     Loop",
    // Сохраняем 32-битный результат R4:R3
    "        LDA     R3",
    "        STA     DW Result",
    "        LDA     R4",
    "        STA     DW Result+2",
    "        HLT",
    ".data",
    "Array1: DW 10, 1, 2, 3, 4, 5, 6, 7000, 8, 9, 10",
    "Array2: DW 10, 1, 2, 3, 4, 5, 6, -7, 8, 9, 10",
    "Result: DD 0"
};

constexpr std::array<std::int16_t, 10> kArr1 = {1, 2, 3, 4, 5, 6, 7000, 8, 9, 10};
constexpr std::array<std::int16_t, 10> kArr2 = {1, 2, 3, 4, 5, 6, -7, 8, 9, 10};

int main() {
    using emulator::Assembler;
    using emulator::Emulator;

    std::cout << emulator::kOperationToString.TryFindBySecond("ADC").has_value() << '\n';

    std::cout << "ASSEMBLING\n";

    Assembler assembler(kArraysConv);
    const auto& assembled = assembler.Assemble();

    std::vector<emulator::MachineWord> machine_code;
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
    for (const auto& pair : assembled.data) {
        emu.load_data(assembled.data_labels.at(pair.first), pair.second);
    }
    emu.load_program(machine_code);

    std::cout << "RUN\n";

    emu.run();

    // std::string input;
    // while(true){
    //     std::cout << "step or stop?: ";
    //     std::cin >> input;
    //     if(input == "stop" || emu.halted()){
    //         break;
    //     }else if(input == "step"){
    //         emu.step();
    //     } else {
    //         continue;
    //     }
    // }


    // for(int i =0; i< 10; ++i){
        auto mem_view = emu.cpu().GetDataMemory()->Read(assembled.data_labels.at("Result"), 4);

        std::int32_t value;
        std::memcpy(&value, mem_view.data(), mem_view.size());

        fmt::print("Result = {}\n", value);

    // }


    std::int32_t result = 0;

    for(int i =0; i < 10; ++i){
        result += kArr1[i]*kArr2[i];
    }

    std::cout << "expected: " << result << ':' << static_cast<std::uint16_t>(result & 0xFFFF) << ' ' << static_cast<uint16_t>((result >> 16) & 0xFFFF);
    return 0;
}
