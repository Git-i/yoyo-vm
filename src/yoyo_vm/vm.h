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
        uint64_t run_code(uint64_t* ip, const Type* arg_begin, size_t arg_size);
        // will default to system alloca
        // can probably change it to use a custom stack
        void* stackalloc(uint64_t size);
    };
}
