import assembler;
import cpu;
import emulator;
import memory;

#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

int main() {
    using cpu::Assembler;
    using cpu::Emulator;

    std::vector<std::string> program_text = {
        "        LDA     #10",
        "        ADD     #5",
        "        SUB     #3",
        "        MUL     #2",
        "        STA     Result",
        "        HLT"
    };

    Assembler assembler;
    const auto assembled = assembler.assemble(program_text);

    std::vector<std::uint16_t> machine_code;
    machine_code.reserve(assembled.size());

    for (const auto& line : assembled) {
        machine_code.push_back(line.machine_code);
        std::cout << line.address << " : " << line.mnemonic
                  << " " << line.operand
                  << " => 0x" << std::hex << line.machine_code << std::dec << '\n';
    }

    Emulator emu(4096, 256);
    emu.load_program(machine_code);

    emu.run();

    std::cout << "\nResult = " << emu.read_data(0) << '\n';

    return 0;
}
