#include "common.h"
#include <span>
#include <string>
namespace Yvm 
{
    class YVM_API Disassembler
    {
        std::string disassemble(std::span<const uint64_t>);
    };
}