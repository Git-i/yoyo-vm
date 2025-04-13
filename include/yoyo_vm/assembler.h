#pragma once
#include "common.h"
#include <vector>
#include <string_view>
#include <cstdint>
#include <unordered_map>

namespace Yvm
{
    class YVM_API Assembler
    {
        std::unordered_map<std::string_view, uint64_t> jump_addrs;
        std::unordered_map<size_t, std::string_view> unresolved_jumps;
    public:
        std::vector<uint64_t> assemble(std::string_view code);
    };
}
