#include "common.h"
#include <span>
#include <string>
#include "vm.h"
namespace Yvm 
{
    class YVM_API Disassembler
    {
    public:
        static std::string disassemble(std::span<const uint64_t>, const VM* vm);
    };
}