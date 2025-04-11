#include <iostream>
#include <vector>
#include <array>
#include "src/yoyo_vm/vm.h"
#include "src/yoyo_vm/instructions.h"
#include "src/yoyo_vm/assembler.h"
#include <chrono>
#include <string.h>
#include <src/yoyo_vm/emitter.h>

inline consteval uint8_t to_u8(OpCode c)
{
    return static_cast<uint8_t>(c);
}

uint8_t constexpr operator "" _u8(const unsigned long long v)
{
    return v;
}
void func(uint32_t val, uint32_t val2)
{
    std::cout << val << " and " << val2 << std::endl;
}

Yvm::VM::Type Yvm::do_native_call(void* function, const VM::Type* begin, size_t arg_size, const void* proto)
{
    reinterpret_cast<void(*)(uint32_t, uint32_t)>(function)(begin[0].u32, begin[1].u32);
    return Yvm::VM::Type{};
}
struct Module
{
    std::vector<uint64_t> code;
    std::unordered_map<std::string, uint64_t*> functions;
};
int main() {
    // n -> 0
    // prev_prev -> 1
    // prev -> 2
    // curr -> 3
    // i -> 4
    Yvm::Emitter em;
    em.write_const<uint32_t>(10);
    em.write_const<uint32_t>(200);
    em.write_const<void*>(nullptr);
    em.write_const(reinterpret_cast<void*>(&func));
    em.write_2b_inst(OpCode::NativeCall, 2);
    em.write_1b_inst(OpCode::Ret);
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
    const32 1
    s_addr 4
    store i32

COND:
    ; i < n
    s_addr 0
    s_addr 4
    load i32
    icmp_lt 32
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
    auto asm_code = std::move(em).get_code();//asma.assemble(c);

    Yvm::VM vm;
    Yvm::VM::Type arg1{ .u32 = 7 };
    auto now = std::chrono::high_resolution_clock::now();
    auto output = vm.run_code(asm_code.data(), &arg1, 1);
    auto end = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::duration<double>>(end - now);
    std::cout << elapsed.count() << "s" << std::endl;
    std::cout << *reinterpret_cast<uint32_t*>(&output) << std::endl;
}
