#include <cstdint>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

#include <emulator/assembler.hpp>
#include <emulator/cpu.hpp>
#include <emulator/emulator.hpp>
#include <emulator/memory.hpp>
#include <emulator/instruction.hpp>
#include "emulator/types.hpp"

int main() {
    using emulator::Assembler;
    using emulator::Emulator;

    std::vector<std::string> program_text = {
        ".program",
        "        LDA     #Result",
        "        ADD     #1",
        "        STA     R0",
        "        LDA     [R0]",
        "        HLT",
        ".data",
        "Result: DB 2, 3, 4"
    };

    std::cout << "ASSEMBLING\n";

    Assembler assembler;
    const auto& assembled = assembler.Assemble(program_text);

    std::vector<std::uint16_t> machine_code;
    machine_code.reserve(assembled.program.size());
    //
    std::cout << "SIZE: " << assembled.program.size() << '\n';

    for (const auto& line : assembled.program) {
        machine_code.push_back(line.machine_code);
        std::cout << line.address << " : " << emulator::kOperationToString.TryFind(line.opcode).value() << " "
                  <<  static_cast<int>(line.mode) << " " << line.operand << " => 0x" << std::hex << line.machine_code
                  << std::dec << '\n';
    }

    Emulator emu(4096, 256);
    std::cout << "LOAD DATA\n";
    for(const auto& pair: assembled.data){
        emu.load_data(assembled.data_labels.at(pair.first).first, pair.second);
    }
    std::cout << "LOAD INSTRUCTIONS\n";
    emu.load_program(machine_code);

    std::cout << "RUN\n";

    emu.run();

    std::cout << "Result = " << emu.read_data(assembled.data_labels.at("Result").first)
              << '\n';

    std::cout << "A = " << emu.get_register(emulator::Register::A) << '\n';

    return 0;
}
