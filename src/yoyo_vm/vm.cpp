#include "vm.h"

#include <exception>
#include <string.h>

#include "instructions.h"

namespace Yvm
{
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
    struct Stack
    {
        std::array<uint64_t, 256> stack{};
        size_t top = 0;
        template<int n> in_t<n> pop()
        {
            return reinterpret_cast<in_t<n>&>(stack[--top]);
        }
        template<int n> sin_t<n> pops()
        {
            return reinterpret_cast<sin_t<n>&>(stack[--top]);
        }
        template<int n> fp_t<n> popf()
        {
            return reinterpret_cast<fp_t<n>&>(stack[--top]);
        }
        template<class T> T*  pop_ptr()
        {
            return reinterpret_cast<T*>(stack[--top]);
        }
        template<typename T>
        void push(const T val)
        {
            reinterpret_cast<T&>(stack[top++]) = val;
        }
    };
    uint64_t VM::run_code(uint64_t* base, const uint64_t* arg_begin, const size_t arg_size) const
    {
        Stack stack;
        // stack data should be trivially copyable
        memcpy(stack.stack.data(), arg_begin, arg_size * sizeof(uint64_t));
        stack.top = arg_size;
        auto ip = reinterpret_cast<OpCode*>(base);
        while (true)
        {
            switch (*ip)
            {
            using enum OpCode;
            case Add8: stack.push<uint8_t>(stack.pop<8>() + stack.pop<8>()); ip++; break;
            case Add16: stack.push<uint16_t>(stack.pop<16>() + stack.pop<16>()); ip++; break;
            case Add32: stack.push<uint32_t>(stack.pop<32>() + stack.pop<32>()); ip++; break;
            case Add64: stack.push<uint64_t>(stack.pop<64>() + stack.pop<64>()); ip++; break;
            case Sub8: stack.push(stack.pop<8>() - stack.pop<8>()); ip++; break;
            case Sub16: stack.push(stack.pop<16>() - stack.pop<16>()); ip++; break;
            case Sub32: stack.push(stack.pop<32>() - stack.pop<32>()); ip++; break;
            case Sub64: stack.push(stack.pop<64>() - stack.pop<64>()); ip++; break;
            case Mul8: stack.push(stack.pop<8>() * stack.pop<8>()); ip++; break;
            case Mul16: stack.push(stack.pop<16>() * stack.pop<16>()); ip++; break;
            case Mul32: stack.push(stack.pop<32>() * stack.pop<32>()); ip++; break;
            case Mul64: stack.push(stack.pop<64>() * stack.pop<64>()); ip++; break;
            case IDiv8: stack.push(stack.pops<8>() / stack.pops<8>()); ip++; break;
            case IDiv16: stack.push(stack.pops<16>() / stack.pops<16>()); ip++; break;
            case IDiv32: stack.push(stack.pops<32>() / stack.pops<32>()); ip++; break;
            case IDiv64: stack.push(stack.pops<64>() / stack.pops<64>()); ip++; break;
            case UDiv8: stack.push(stack.pop<8>() / stack.pop<8>()); ip++; break;
            case UDiv16: stack.push(stack.pop<16>() / stack.pop<16>()); ip++; break;
            case UDiv32: stack.push(stack.pop<32>() / stack.pop<32>()); ip++; break;
            case UDiv64: stack.push(stack.pop<64>() / stack.pop<64>()); ip++; break;
            case IRem8: stack.push(stack.pops<8>() % stack.pops<8>()); ip++; break;
            case IRem16: stack.push(stack.pops<16>() % stack.pops<16>()); ip++; break;
            case IRem32: stack.push(stack.pops<32>() % stack.pops<32>()); ip++; break;
            case IRem64: stack.push(stack.pops<64>() % stack.pops<64>()); ip++; break;
            case URem8: stack.push(stack.pop<8>() % stack.pop<8>()); ip++; break;
            case URem16: stack.push(stack.pop<16>() % stack.pop<16>()); ip++; break;
            case URem32: stack.push(stack.pop<32>() % stack.pop<32>()); ip++; break;
            case URem64: stack.push(stack.pop<64>() % stack.pop<64>()); ip++; break;
            case FAdd32: stack.push(stack.pop<32>() + stack.pop<32>()); ip++; break;
            case FAdd64: stack.push(stack.pop<64>() + stack.pop<64>()); ip++; break;
            case FSub32: stack.push(stack.pop<32>() - stack.pop<32>()); ip++; break;
            case FSub64: stack.push(stack.pop<64>() - stack.pop<64>()); ip++; break;
            case FMul32: stack.push(stack.pop<32>() * stack.pop<32>()); ip++; break;
            case FMul64: stack.push(stack.pop<64>() * stack.pop<64>()); ip++; break;
            case FDiv32: stack.push(stack.pop<32>() / stack.pop<32>()); ip++; break;
            case FDiv64: stack.push(stack.pop<64>() / stack.pop<64>()); ip++; break;
            case Jump:
            case JumpIf:
                ip++; break;
            case Ret: return stack.pop<64>();
            case RetVoid: return 0;
            case Constant8: stack.push(static_cast<uint8_t>(*++ip)); ip++; break;
            case Constant16:
                {
                    // there may be padding bytes because it must be 2 byte aligned
                    auto offset =
                        reinterpret_cast<uint8_t*>(++ip) -
                        reinterpret_cast<uint8_t*>(base);
                    ip += (2 - offset) % 2;
                    stack.push(*reinterpret_cast<uint16_t*>(ip));
                    ip += 2; break;
                }
            case Constant32:
                {
                    // there may be padding bytes because it must be 4 byte aligned
                    auto offset =
                        reinterpret_cast<uint8_t*>(++ip) -
                        reinterpret_cast<uint8_t*>(base);
                    ip += (4 - offset) % 4;
                    stack.push(*reinterpret_cast<uint32_t*>(ip));
                    ip += 4; break;
                }
            case Constant64:
                {
                    auto offset =
                        reinterpret_cast<uint8_t*>(++ip) -
                        reinterpret_cast<uint8_t*>(base);
                    ip += (8 - offset) % 8;
                    stack.push(*reinterpret_cast<uint64_t*>(ip));
                    ip += 8; break;
                }
            case StackAddr: stack.push(stack.stack[static_cast<uint8_t>(*++ip)]); ip++; break;
            case Call:
                {
                    auto code = stack.pop_ptr<uint64_t>();
                    auto arg_size_new = static_cast<size_t>(*++ip);
                    auto arg_begin_new = stack.stack.data() + stack.top - arg_size_new;
                    run_code(code, arg_begin_new, arg_size_new);
                }
            case Jump:
                {

                }
            }

        }
    }

}
