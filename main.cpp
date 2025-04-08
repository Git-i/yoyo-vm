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
    // n -> 0
    // prev_prev -> 1
    // prev -> 2
    // curr -> 3
    // i -> 4
    auto code = std::array{
        to_u8(OpCode::AllocaConst),4_u8,
        to_u8(OpCode::AllocaConst),4_u8,
        to_u8(OpCode::AllocaConst),4_u8,
        to_u8(OpCode::AllocaConst),4_u8,
        //---------------------------
        //initialize
        to_u8(OpCode::Constant32), 0_u8,
        0_u8, 0_u8,0_u8, 0_u8, 0_u8, 0_u8,
        //------------------------------
        to_u8(OpCode::StackAddr), 2_u8,
        to_u8(OpCode::Store), 2_u8,
        to_u8(OpCode::Constant32), 0_u8,0_u8, 0_u8,
        //------------------------------
        0_u8, 0_u8, 0_u8, 0_u8,
        to_u8(OpCode::StackAddr), 3_u8,
        to_u8(OpCode::Store), 2_u8,
        //---------------------------------
        to_u8(OpCode::Constant8), 1_u8,
        to_u8(OpCode::StackAddr), 4_u8,
        to_u8(OpCode::Store), 2_u8,
        // loop condition check
        to_u8(OpCode::StackAddr), 4_u8,
        //---------------------------------
        to_u8(OpCode::Load), 2_u8,
        to_u8(OpCode::StackAddr), 0_u8,
        to_u8(OpCode::ICmpGt), 32_u8,
        to_u8(OpCode::Constant64), 0_u8,
        //--------------------------------
        0_u8, 0_u8, 0_u8, 0_u8, 0_u8, 0_u8, 0_u8, 0_u8, // position 6
        //--------------------------------
        to_u8(OpCode::JumpIfFalse),
        // loop body
        to_u8(OpCode::StackAddr), 2_u8,
        to_u8(OpCode::Load), 2_u8,
        to_u8(OpCode::StackAddr), 1_u8,
        to_u8(OpCode::Store), 2_u8,

        to_u8(OpCode::StackAddr), 3_u8,
        to_u8(OpCode::Load), 2_u8,
        to_u8(OpCode::StackAddr), 2_u8,
        to_u8(OpCode::Store), 2_u8,

        to_u8(OpCode::StackAddr), 2_u8,
        to_u8(OpCode::Load), 2_u8,
        to_u8(OpCode::StackAddr), 1_u8,
        to_u8(OpCode::Load), 2_u8,

        to_u8(OpCode::Add32),

        to_u8(OpCode::StackAddr), 3_u8,
        to_u8(OpCode::Store), 2_u8,

        to_u8(OpCode::StackAddr), 4_u8,
        //----------------------------------
        to_u8(OpCode::Load), 2_u8,
        to_u8(OpCode::Constant32), 0_u8,
        //----------------
        0_u8, 0_u8, 0_u8, 0_u8, // should be one
        //-----------------
        to_u8(OpCode::Add32),
        to_u8(OpCode::StackAddr), 4_u8,
        to_u8(OpCode::Store), 2_u8,

        to_u8(OpCode::Constant64), 0_u8, 0_u8,
        //----------------------------
        0_u8, 0_u8, 0_u8, 0_u8, 0_u8, 0_u8, 0_u8, 0_u8,
        //----------------------------
        to_u8(OpCode::Jump),

        to_u8(OpCode::StackAddr), 3_u8,
        to_u8(OpCode::Load), 2_u8,
        to_u8(OpCode::Ret)
    };
    uint64_t a[15];
    memcpy(&a, code.data(), code.size());
    //set constants-----------------
    a[6] = code.size() - 5;
    a[13] = 38;
    reinterpret_cast<uint32_t*>(a)[6] = 1;
    ((uint32_t*)a)[23] = 1;
    //-----------------------------
    Yvm::VM vm;
    for (uint32_t i = 1; i < 15; i++)
    {
        Yvm::VM::Type arg1{ .u32 = i };
        auto output = vm.run_code(a, &arg1, 1);
        auto value = *reinterpret_cast<int32_t*>(&output);
        std::cout << value << std::endl;
    }

}
