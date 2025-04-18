#pragma once
#include "common.h"
#include <cstdint>
#include <array>
#include <string>
#include <unordered_map>
#include <vector>

#include "instructions.h"

namespace Yvm
{
    class VMRunner;
    /// Class representing a virtual machine
    /// note that the vm does not own any code, that would be done by a module system

    class YVM_API Module
    {
    public:
        std::vector<uint64_t> code;
        std::unordered_map<uint64_t*, std::string> functions;
    };
    class YVM_API VM
    {
        std::vector<Module*> registered_modules;
        friend class VMRunner;
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
        /// Construct a @link VMRunner instance
        VMRunner new_runner();
    };
    /// This holds necessary state required to run code
    /// you can have as many instances of this running at the same time (say on different threads)
    class YVM_API VMRunner
    {
        explicit VMRunner(const VM& vm): vm(vm) {};
        const VM& vm;
        std::unordered_map<void*, uint64_t*> registered_objects;
    public:
        VMRunner(VMRunner&&) noexcept = default;

        VM::Type run_code(uint64_t* ip, const VM::Type* arg_begin, size_t arg_size, size_t stack_off = 0);
        // will default to system alloca
        // can probably change it to use a custom stack
        void* stackalloc(uint64_t size);
    private:
        std::array<VM::Type, 256> stack_data{};
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
