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
    template<int n>
    struct in {};
    template<int n>
    struct sin {};
    template<int n>
    struct fp {};
    template<> struct in<8> { using type = uint8_t; };
    template<> struct in<16> { using type = uint16_t; };
    template<> struct in<32> { using type = uint32_t; };
    template<> struct in<64> { using type = uint64_t; };

    template<> struct sin<8> { using type = int8_t; };
    template<> struct sin<16> { using type = int16_t; };
    template<> struct sin<32> { using type = int32_t; };
    template<> struct sin<64> { using type = int64_t; };

    template<> struct fp<32> { using type = float; };
    template<> struct fp<64> { using type = double; };
    template<int n> using in_t = typename in<n>::type;
    template<int n> using sin_t = typename sin<n>::type;
    template<int n> using fp_t = typename fp<n>::type;
    struct Stack;
    class YVM_API Module
    {
    public:
        std::unordered_map<std::string, std::vector<uint64_t>> code;
        std::unordered_map<void**, std::string> unresolved_externals;
    };
    class YVM_API VM
    {
        std::vector<Module*> registered_modules;
        std::list<std::string> strings;
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
        /// Implement this if you want to support native calls
        /// @param function the pointer to the function
        /// @param begin is start of the argument list, this pointer points to data within the vm stack
        ///
        /// @param arg_size this is number of arguments pointed to by @p begin
        ///
        /// @param proto this is a user defined pointer from the stack top after @p function is removed,
        /// it can be anything, it's called @c proto because the common use case is to store a description to the function's
        /// prototype
        VM::Type(*do_native_call)(void* function, VM::Type* begin, size_t arg_size, void* proto);
        void(*intrinsic_handler)(Stack& stack, uint8_t instrinsic_number);
        /// Construct a @link VMRunner instance
        VMRunner new_runner();
        /// Link all registered modules and resolve external symbols
        /// Returns a list of unresolved symbols if any
        std::vector<std::string> link();
        void add_module(Module* module);
        const char* add_string(std::string str);
        bool is_registered_string(const char*) const;
        std::string name_of(void* ptr) const;
    };
    struct Stack
    {

        VM::Type* stack;
        size_t top = 0;
        template<int n> in_t<n> pop()
        {
            if constexpr (n == 8) return stack[--top].u8;
            else if constexpr (n == 16) return stack[--top].u16;
            else if constexpr (n == 32) return stack[--top].u32;
            else if constexpr (n == 64) return stack[--top].u64;
            else static_assert(false, "Invalid integer size");
        }
        template<int n> sin_t<n> pops()
        {
            if constexpr (n == 8) return stack[--top].i8;
            else if constexpr (n == 16) return stack[--top].i16;
            else if constexpr (n == 32) return stack[--top].i32;
            else if constexpr (n == 64) return stack[--top].i64;
            else static_assert(false, "Invalid integer size");
        }
        template<int n> fp_t<n> popf()
        {
            if constexpr (n == 32) return stack[--top].f32;
            else if constexpr (n == 64) return stack[--top].f64;
            else static_assert(false, "Invalid integer size");
        }
        VM::Type pop_raw()
        {
            return stack[--top];
        }
        template<class T> T* pop_ptr()
        {
            return static_cast<T*>(stack[--top].ptr);
        }
        template<class T> T* peek_ptr()
        {
            return static_cast<T*>(stack[top - 1].ptr);
        }
        template<typename T>
        void push(const T val)
        {
            auto& top_ref = stack[top++];
            if constexpr (std::is_same_v<T, int8_t>) top_ref.i8 = val;
            else if constexpr (std::is_same_v<T, int16_t>) top_ref.i16 = val;
            else if constexpr (std::is_same_v<T, int32_t>) top_ref.i32 = val;
            else if constexpr (std::is_same_v<T, int64_t>) top_ref.i64 = val;
            else if constexpr (std::is_same_v<T, uint8_t>) top_ref.u8 = val;
            else if constexpr (std::is_same_v<T, uint16_t>) top_ref.u16 = val;
            else if constexpr (std::is_same_v<T, uint32_t>) top_ref.u32 = val;
            else if constexpr (std::is_same_v<T, uint64_t>) top_ref.u64 = val;
            else if constexpr (std::is_same_v<T, float>) top_ref.f32 = val;
            else if constexpr (std::is_same_v<T, double>) top_ref.f64 = val;
            else if constexpr (std::is_pointer_v<T>) top_ref.ptr = val;
            else if constexpr (std::is_same_v<T, VM::Type>) memcpy(&top_ref, &val, sizeof(T));
            else static_assert(false, "Invalid push type");
        }
    };
    /// This holds necessary state required to run code
    /// you can have as many instances of this running at the same time (say on different threads)
    class YVM_API VMRunner
    {
        friend class VM;
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
}
