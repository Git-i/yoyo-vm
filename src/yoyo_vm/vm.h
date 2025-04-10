#pragma once
#include <cstdint>
#include <span>

#include "instructions.h"

namespace Yvm
{
    class VM
    {
    public:
        union Type
        {
            uint8_t u8; int8_t i8;
            uint16_t u16; int16_t i16;
            uint32_t u32; int32_t i32;
            uint64_t u64; int64_t i64;
            float f32; double f64;
            void* ptr;
        };
        uint64_t run_code(uint64_t* ip, const Type* arg_begin, size_t arg_size, size_t stack_off = 0);
        // will default to system alloca
        // can probably change it to use a custom stack
        void* stackalloc(uint64_t size);
    private:
        std::array<Type, 256> stack_data{};
    };
    /// Implement this if you want to support native calls
    /// @param function the pointer to the function
    /// @param begin is start of the argument list, this pointer points to data within the vm stack
    ///
    /// @param arg_size this is number of arguments pointed to by @p begin
    ///
    /// @param proto this is a user defined pointer from the stack top after @p function is removed,
    /// it can be anything, it's called @c proto because the common use case is to store a description to the function's
    /// prototype
    extern VM::Type do_native_call(void* function, const VM::Type* begin, size_t arg_size, const void* proto);
}
