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


constexpr std::array<std::string_view, 33> kArraysConv = {
".program",
"        LDA     Array1", // берем размер массива
"        STA     R0", // записываем в RO
"        LDA     #Array1+1", // берем адрес первого элемента
"        STA     R1", // грузим адрес в R1
"        LDA     #Array2+1", //берем адрес первого элемента
"        STA     R2", // грузим адрес в R2
"        LDA     #0", // Обнулям Акк
"        STA     R3", // Уписываем в R2 регистр суммы
"Loop:   LDA     DW [R1]", // Читаем по адресу в памяти (we can additionaly describe how much bytes we can read)
"        STA     R5", // сохраняем в регистр значение
"        LDA     DW [R2]", // грузим элемент второго массива (we can additionaly describe how much bytes we can read)
"        MUL     R5", // умножаем на число из Array1
"        ADC     R3", //  складываем с текущей суммой
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
"Array1: DW 10, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 9000, 10000",
"Array2: DW 10, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 9000, 10000",
"Result: DD 0"
};

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
    //
    auto mem_view = emu.cpu().data_memory()->Read(assembled.data_labels.at("Result"), 2);

    std::uint16_t value;
    std::memcpy(&value, mem_view.data(), mem_view.size());

    std::cout << "Result = "
              << value<< '\n';


    return 0;
}
