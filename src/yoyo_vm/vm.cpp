#include "vm.h"
#include <array>
#include <cassert>
#include <exception>
#include <cstring>

#include "instructions.h"
#define FP_OP(PUSH_TY, OP) switch (static_cast<uint8_t>(*++ip))\
{\
case 32: stack.push<PUSH_TY>(stack.popf<32>() OP stack.popf<32>()); break;\
case 64: stack.push<PUSH_TY>(stack.popf<64>() OP stack.popf<64>()); break;\
default: break;\
}\
ip++; break;
#define UNSIGNED_OP(PUSH_TY1, PUSH_TY2, PUSH_TY3, PUSH_TY4, OP) switch (static_cast<uint8_t>(*++ip))\
                    {\
                        case  8: stack.push<PUSH_TY1>(stack.pop<8>() OP stack.pop<8>()); break;\
                        case 16: stack.push<PUSH_TY2>(stack.pop<16>() OP stack.pop<16>()); break;\
                        case 32: stack.push<PUSH_TY3>(stack.pop<32>() OP stack.pop<32>()); break;\
                        case 64: stack.push<PUSH_TY4>(stack.pop<64>() OP stack.pop<64>()); break;\
                        default: break;\
                    }\
ip++; break;
//unsigned op same type
#define UNSIGNED_OP_ST(PUSH_TY, OP) UNSIGNED_OP(PUSH_TY, PUSH_TY, PUSH_TY, PUSH_TY, OP)
//unsigned op corresponding type
#define UNSIGNED_OP_CT(OP) UNSIGNED_OP(uint8_t, uint16_t, uint32_t, uint32_t, OP)
#define SIGNED_OP(PUSH_TY1, PUSH_TY2, PUSH_TY3, PUSH_TY4, OP) switch (static_cast<uint8_t>(*++ip))\
{\
case  8: stack.push<PUSH_TY1>(stack.pops<8>() OP stack.pops<8>()); break;\
case 16: stack.push<PUSH_TY2>(stack.pops<16>() OP stack.pops<16>()); break;\
case 32: stack.push<PUSH_TY3>(stack.pops<32>() OP stack.pops<32>()); break;\
case 64: stack.push<PUSH_TY4>(stack.pops<64>() OP stack.pops<64>()); break;\
default: break;\
}\
ip++; break;
#define SIGNED_OP_ST(PUSH_TY, OP) SIGNED_OP(PUSH_TY, PUSH_TY, PUSH_TY, PUSH_TY, OP)
#define SIGNED_OP_CT(OP) SIGNED_OP(int8_t, int16_t, int32_t, int32_t, OP)

#ifndef alloca
#define alloca(x) stackalloc(x)
#endif
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
        template<class T> T*  pop_ptr()
        {
            return static_cast<T*>(stack[--top].ptr);
        }
        template<class T> T*  peek_ptr()
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
    VM::Type VMRunner::run_code(uint64_t* base, const VM::Type* arg_begin, const size_t arg_size, size_t stack_off)
    {
        Stack stack{ stack_data.data() + stack_off, 0 };
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
            case Ret: return stack.pop_raw();
            case RetVoid: return VM::Type{ .u64 = 0 };
            case Constant8: stack.push(static_cast<uint8_t>(*++ip)); ip++; break;
            case Constant16:
                {
                    // there may be padding bytes because it must be 2 byte aligned
                    auto offset =
                        reinterpret_cast<uint8_t*>(++ip) -
                        reinterpret_cast<uint8_t*>(base);
                    ip += (2 - offset % 2) % 2;
                    stack.push(*reinterpret_cast<uint16_t*>(ip));
                    ip += 2; break;
                }
            case Constant32:
                {
                    // there may be padding bytes because it must be 4 byte aligned
                    auto offset =
                        reinterpret_cast<uint8_t*>(++ip) -
                        reinterpret_cast<uint8_t*>(base);
                    ip += (4 - offset % 4) % 4;
                    stack.push(*reinterpret_cast<uint32_t*>(ip));
                    ip += 4; break;
                }
            case Constant64:
                {
                    auto offset =
                        reinterpret_cast<uint8_t*>(++ip) -
                        reinterpret_cast<uint8_t*>(base);
                    ip += (8 - offset % 8) % 8;
                    stack.push(*reinterpret_cast<uint64_t*>(ip));
                    ip += 8; break;
                }
            case StackAddr: stack.push(stack.stack[static_cast<uint8_t>(*++ip)]); ip++; break;
            case Call:
                {
                    auto code = stack.pop_ptr<uint64_t>();
                    auto arg_size_new = static_cast<size_t>(*++ip);
                    auto arg_begin_new = stack.stack + stack.top - arg_size_new;
                    auto new_top = stack.top - arg_size_new;
                    auto val = run_code(code, arg_begin_new, arg_size_new, stack_off + stack.top - arg_size_new);
                    stack.top = new_top;
                    stack.push(val);
                    break;
                }
            case Jump:  ip = reinterpret_cast<OpCode*>(base) + stack.pop<64>(); break;
            case CmpEq: { UNSIGNED_OP_ST(uint8_t, ==) }
            case CmpNe: { UNSIGNED_OP_ST(uint8_t, !=) }
            case UCmpGt: { UNSIGNED_OP_ST(uint8_t, >) }
            case UCmpGe: { UNSIGNED_OP_ST(uint8_t, >=) }
            case UCmpLt: { UNSIGNED_OP_ST(uint8_t, <) }
            case UCmpLe: { UNSIGNED_OP_ST(uint8_t, <=) }
            case ICmpGt: { SIGNED_OP_ST(uint8_t, >) }
            case ICmpGe: { SIGNED_OP_ST(uint8_t, >=) }
            case ICmpLt: { SIGNED_OP_ST(uint8_t, <) }
            case ICmpLe: { SIGNED_OP_ST(uint8_t, <=) }
            case Shl: { UNSIGNED_OP_CT(<<) }
            case Shr: { UNSIGNED_OP_CT(>>) }
            case BitAnd: { UNSIGNED_OP_CT(&) }
            case BitOr: { UNSIGNED_OP_CT(|) }
            case BitXor: { UNSIGNED_OP_CT(^) }
            case Alloca: stack.push(alloca(stack.pop<32>())); ip++; break;
            case AllocaConst: stack.push(alloca(static_cast<uint8_t>(*++ip))); ip++; break;
            case Load:
                {
                    auto ptr = stack.pop_ptr<void>();
                    switch (static_cast<uint8_t>(*++ip))
                    {
                    case 0: stack.push(*static_cast<int8_t*>(ptr)); break;
                    case 1: stack.push(*static_cast<int16_t*>(ptr)); break;
                    case 2: stack.push(*static_cast<int32_t*>(ptr)); break;
                    case 3: stack.push(*static_cast<int64_t*>(ptr)); break;
                    case 4: stack.push(*static_cast<uint8_t*>(ptr)); break;
                    case 5: stack.push(*static_cast<uint16_t*>(ptr)); break;
                    case 6: stack.push(*static_cast<uint32_t*>(ptr)); break;
                    case 7: stack.push(*static_cast<uint64_t*>(ptr)); break;
                    case 8: stack.push(*static_cast<float*>(ptr)); break;
                    case 9: stack.push(*static_cast<double*>(ptr)); break;
                    case 10: stack.push(ptr); break;
                    }
                    ip++; break;
                }
            case Store:
                {
                    auto ptr = stack.pop_ptr<void>();
                    switch (static_cast<uint8_t>(*++ip))
                    {
                    case 0: *static_cast<int8_t*>(ptr) = stack.pops<8>(); break;
                    case 1: *static_cast<int16_t*>(ptr) = stack.pops<16>(); break;
                    case 2: *static_cast<int32_t*>(ptr) = stack.pops<32>(); break;
                    case 3: *static_cast<int64_t*>(ptr) = stack.pops<64>(); break;
                    case 4: *static_cast<uint8_t*>(ptr) = stack.pop<8>(); break;
                    case 5: *static_cast<uint16_t*>(ptr) = stack.pop<16>(); break;
                    case 6: *static_cast<uint32_t*>(ptr) = stack.pop<32>(); break;
                    case 7: *static_cast<uint64_t*>(ptr) = stack.pop<64>(); break;
                    case 8: *static_cast<float*>(ptr) = stack.popf<32>(); break;
                    case 9: *static_cast<double*>(ptr) = stack.popf<64>(); break;
                    case 10: *static_cast<void**>(ptr) = stack.pop_ptr<void>(); break;
                    }
                    ip++; break;
                }
            case PtrOff: stack.push(stack.pop_ptr<uint8_t>() + stack.pop<64>()); ip++; break;
            case FNeg32: stack.push(-stack.popf<32>()); ip++; break;
            case FNeg64: stack.push(-stack.popf<64>()); ip++; break;
            case Panic: break;
            case PtrOffConst: stack.push(stack.pop_ptr<uint8_t>() + static_cast<uint8_t>(*++ip)); ip++; break;
            case UConv:
                {
#define UNSIGNED_FROM_SWITCH(CODE)\
    switch (from) {\
        case 8: CODE (stack.pop<8>()); break;\
        case 16: CODE (stack.pop<16>()); break;\
        case 32: CODE (stack.pop<32>()); break;\
        case 64: CODE (stack.pop<64>()); break;\
    }
                    auto from = static_cast<uint8_t>(*++ip);
                    auto to = static_cast<uint8_t>(*++ip);
                    switch (to)
                    {
                    case 8: UNSIGNED_FROM_SWITCH(stack.push<uint8_t>); break;
                    case 16: UNSIGNED_FROM_SWITCH(stack.push<uint16_t>) break;
                    case 32: UNSIGNED_FROM_SWITCH(stack.push<uint32_t>) break;
                    case 64: UNSIGNED_FROM_SWITCH(stack.push<uint64_t>) break;
                    }
                    ip++; break;
                }
            case SConv:
                {
#define SIGNED_FROM_SWITCH(CODE)\
    switch (from) {\
    case 8: CODE (stack.pops<8>()); break;\
    case 16: CODE (stack.pops<16>()); break;\
    case 32: CODE (stack.pops<32>()); break;\
    case 64: CODE (stack.pops<64>()); break;\
    }
                    auto from = static_cast<uint8_t>(*++ip);
                    auto to = static_cast<uint8_t>(*++ip);
                    switch (to)
                    {
                    case 8: UNSIGNED_FROM_SWITCH(stack.push<int8_t>); break;
                    case 16: UNSIGNED_FROM_SWITCH(stack.push<int16_t>) break;
                    case 32: UNSIGNED_FROM_SWITCH(stack.push<int32_t>) break;
                    case 64: UNSIGNED_FROM_SWITCH(stack.push<int64_t>) break;
                    }
                    ip++; break;
                }
            case FpConv:
                {
#define FP_FROM_SWITCH(CODE)\
    switch (from) {\
    case 32: CODE (stack.popf<32>()); break;\
    case 64: CODE (stack.popf<64>()); break;\
    }
                    auto from = static_cast<uint8_t>(*++ip);
                    auto to = static_cast<uint8_t>(*++ip);
                    switch (to)
                    {
                    case 32: FP_FROM_SWITCH(stack.push<float>) break;
                    case 64: FP_FROM_SWITCH(stack.push<double>) break;
                    }
                    ip++; break;
                }
            case FpToSi:
                break;
            case FpToUi:
                break;
            case UiToFp:
                break;
            case SiToFp:
                break;
            case JumpIfFalse:
                {
                    auto off = stack.pop<64>();
                    if (stack.pop<8>() == 0) ip = reinterpret_cast<OpCode*>(base) + off;
                    else ip++;
                    break;
                }
            case NativeCall:
                {
                    auto code = stack.pop_ptr<void>();
                    auto proto = stack.pop_ptr<const void>();
                    auto arg_size_new = static_cast<size_t>(*++ip);
                    auto arg_begin_new = stack.stack + stack.top - arg_size_new;
                    stack.top -= arg_size_new;
                    auto val = do_native_call(code, arg_begin_new, arg_size_new, proto);
                    stack.push(val);
                    break;
                }
            case RegObj:
                {
                    auto destructor = stack.pop_ptr<uint64_t>();
                    auto obj = stack.peek_ptr<void>();
                    registered_objects[obj] = destructor;
                    break;
                }
            case CheckReg:
                {
                    auto obj = stack.peek_ptr<void>();
                    stack.push<uint8_t>(registered_objects.contains(obj));
                    break;
                }
            case PopReg:
                {
                    auto obj = stack.pop_ptr<void>();
                    if (auto reg_it = registered_objects.find(obj); reg_it != registered_objects.end())
                    {
                        registered_objects.erase(reg_it);
                        stack.push<uint8_t>(1);
                    } else stack.push<uint8_t>(0);
                    break;
                }
            case Pop: stack.top--; break;
            case FCmpEq: { FP_OP(uint8_t, ==) }
            case FCmpNe: { FP_OP(uint8_t, !=) }
            case FCmpGt: { FP_OP(uint8_t, >) }
            case FCmpGe: { FP_OP(uint8_t, >=) }
            case FCmpLt: { FP_OP(uint8_t, <) }
            case FCmpLe: { FP_OP(uint8_t, <=) }
            }

        }
    }

}
