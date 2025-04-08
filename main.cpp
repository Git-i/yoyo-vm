#include <iostream>
#include <vector>

#include "src/yoyo_vm/vm.h"
#include "src/yoyo_vm/instructions.h"

inline consteval uint8_t to_u8(OpCode c)
{
    return static_cast<uint8_t>(c);
}

uint8_t constexpr operator "" _u8(const unsigned long long v)
{
    return v;
}
int main() {
    auto add_u8s = std::array{
        //u8s should already be atop the stack
        to_u8(OpCode::Add8),
        to_u8(OpCode::Ret)
    };
    uint64_t add_u8_code;
    memcpy(&add_u8_code, add_u8s.data(), add_u8s.size());
    auto code = std::array{
        to_u8(OpCode::Constant8), 100_u8,
        to_u8(OpCode::Constant8), 150_u8,
        to_u8(OpCode::Constant64), 0_u8,0_u8,0_u8,  /*here*/ 0_u8,0_u8,0_u8,0_u8,0_u8,0_u8,0_u8,0_u8,
        to_u8(OpCode::Call), 2_u8,
        to_u8(OpCode::Ret)
    };
    uint64_t a[3];
    memcpy(&a, code.data(), code.size());
    a[1] = reinterpret_cast<uint64_t>(&add_u8_code);
    Yvm::VM vm;
    auto output = vm.run_code(a, nullptr, 0);
    auto value = *reinterpret_cast<uint8_t*>(&output);
    std::cout << uint32_t{value} << std::endl;
}
