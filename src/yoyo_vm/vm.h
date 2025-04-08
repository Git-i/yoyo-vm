#pragma once
#include <cstdint>
#include <span>

#include "instructions.h"

namespace Yvm
{
    class VM
    {
    public:
        uint64_t run_code(uint64_t* ip, const uint64_t* arg_begin, size_t arg_size) const;
    };
}
