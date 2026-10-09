import architecture;

#include <iostream>

int main() {
    using cpu::ALU;

    const auto add = ALU::add16(12, -7);
    const auto sub = ALU::sub16(12, 7);
    const auto mul = ALU::mul16(5, -3);
    const auto longmul = ALU::mul32(30000, 30000);

    std::cout << "add16(12, -7) = " << add.value << " | zero=" << add.zero
              << " neg=" << add.negative << " carry=" << add.carry
              << " overflow=" << add.overflow << '\n';

    std::cout << "sub16(12, 7) = " << sub.value << '\n';
    std::cout << "mul16(5, -3) = " << mul.value << '\n';
    std::cout << "mul32(30000, 30000) = " << longmul.asInt32() << '\n';

    return 0;
}
