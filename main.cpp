#include <iostream>
#include <vector>
#include <array>
#include "src/yoyo_vm/vm.h"
#include "src/yoyo_vm/instructions.h"
#include "src/yoyo_vm/assembler.h"
#include <chrono>
#include <string.h>

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
    auto c = R"(
    ; allocate prev_prev, prev, curr and i
    alloc_const 4
    alloc_const 4
    alloc_const 4
    alloc_const 4
    ; set prev to 3
    const32 0
    s_addr 2
    store i32
    ; set curr to 1
    const32 1
    s_addr 3
    store i32
    ; set i to 1
    const8 1
    s_addr 4
    store i32

COND:
    ; i < n
    s_addr 4
    load i32
    s_addr 0
    icmp_gt 32
    jump_if_f END

    ; loop body
    s_addr 2
    load i32
    s_addr 1
    store i32

    s_addr 3
    load i32
    s_addr 2
    store i32

    ; prev_prev + prev
    s_addr 2
    load i32
    s_addr 1
    load i32
    add32

    s_addr 3
    store i32

    ; i++
    s_addr 4
    load i32
    const32 1
    add32
    s_addr 4
    store i32

    jump COND
END:
    ; return curr
    s_addr 3
    load i32
    ret
)";
    Yvm::Assembler asma;
    auto asm_code = asma.assemble(c);
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
    Yvm::VM::Type arg1{ .u32 = 7 };
    auto now = std::chrono::high_resolution_clock::now();
    auto output = vm.run_code(asm_code.data(), &arg1, 1);
    auto end = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::duration<double>>(end - now);
    std::cout << elapsed.count() << "s" << std::endl;
    std::cout << *reinterpret_cast<uint32_t*>(&output) << std::endl;
}
