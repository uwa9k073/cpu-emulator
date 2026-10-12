// #include <cstdint>
#include <iostream>
#include <string>
#include <vector>

import assembler;
import cpu;
import emulator;
import memory;
import instruction;

int main() {
    using cpu::Assembler;
    using cpu::Emulator;
    using cpu::InstructionSet;

    std::vector<std::string> program_text = {
        "        LDA     #10",
        "        ADD     #5",
        "        SUB     #3",
        "        MUL     #2",
        "        STA     Result",
        "        HLT",
        ".data",
        "Result: DW 0"
    };

    Assembler assembler;
    const cpu::AssemblerOutput& assembled = assembler.assemble(program_text);

    std::vector<std::uint16_t> machine_code;
    machine_code.reserve(assembled.program.size());

    for (const auto& line : assembled.program) {
        machine_code.push_back(line.machine_code);
        std::cout << line.address << " : " << InstructionSet::to_string(line.opcode) << " " << line.operand
                  << " => 0x" << std::hex << line.machine_code << std::dec << '\n';
    }

    Emulator emu(4096, 256);
    emu.load_program(machine_code);

    emu.run();

    std::cout << "\nResult = " << emu.read_data(assembled.data_labels.at("Result"))
              << '\n';

    return 0;
}
