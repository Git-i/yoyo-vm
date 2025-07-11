#include "yoyo_vm/disassembler.h"
#include "yoyo_vm/instructions.h"
#include <cstdint>
#include <format>
#include <string>
#include <array>
namespace Yvm 
{
    std::string Disassembler::disassemble(std::span<const uint64_t> insts, const VM* vm) {
        auto ip = reinterpret_cast<const uint8_t*>(insts.data());
        std::string asm_code;
        auto base = ip;
        auto ip_begin = ip;
        auto write_line = [&asm_code, &ip_begin, base](std::string code) {
            asm_code += std::format("{:0>10}: {}\n", ip_begin - base, code);
        };
        auto load_arr = std::array{
            "i8",
            "i16",
            "i32",
            "i64",
            "u8",
            "u16",
            "u32",
            "u64",
            "f32",
            "f64",
            "ptr",
        };
        auto load_str = [&load_arr](uint8_t val) {
            if(val >= load_arr.size()) return "inv";
            return load_arr[val];
        };
        while(
            (ip - base) < 
            static_cast<std::ptrdiff_t>(insts.size() * 8)) {
            ip_begin = ip;
            switch (static_cast<OpCode>(*ip)) {
            case OpCode::Nop: write_line("nop"); ip++; break;
            case OpCode::Add8: write_line("add8");     ip++; break;
            case OpCode::Add16: write_line("add16");   ip++; break;
            case OpCode::Add32: write_line("add32");   ip++; break;
            case OpCode::Add64: write_line("add64");   ip++; break;
            case OpCode::Sub8: write_line("sub8");     ip++; break;
            case OpCode::Sub16: write_line("sub16");   ip++; break;
            case OpCode::Sub32: write_line("sub32");   ip++; break;
            case OpCode::Sub64: write_line("sub64");   ip++; break;
            case OpCode::Mul8: write_line("mul8");     ip++; break; 
            case OpCode::Mul16: write_line("mul16");   ip++; break;
            case OpCode::Mul32: write_line("mul32");   ip++; break;
            case OpCode::Mul64: write_line("mul64");   ip++; break;
            case OpCode::IDiv8: write_line("idiv8");   ip++; break;
            case OpCode::IDiv16: write_line("idiv16"); ip++; break;
            case OpCode::IDiv32: write_line("idiv32"); ip++; break;
            case OpCode::IDiv64: write_line("idiv64"); ip++; break;
            case OpCode::UDiv8: write_line("udiv8");   ip++; break;
            case OpCode::UDiv16: write_line("udiv16"); ip++; break;
            case OpCode::UDiv32: write_line("udiv32"); ip++; break;
            case OpCode::UDiv64: write_line("udiv64"); ip++; break;
            case OpCode::IRem8: write_line("irem8");   ip++; break;
            case OpCode::IRem16: write_line("irem16"); ip++; break;
            case OpCode::IRem32: write_line("irem32"); ip++; break;
            case OpCode::IRem64: write_line("irem64"); ip++; break;
            case OpCode::URem8: write_line("urem8");   ip++; break;
            case OpCode::URem16: write_line("urem16"); ip++; break;
            case OpCode::URem32: write_line("urem32"); ip++; break;
            case OpCode::URem64: write_line("urem64"); ip++; break;
            case OpCode::FAdd32: write_line("fadd32"); ip++; break;
            case OpCode::FAdd64: write_line("fadd64"); ip++; break;
            case OpCode::FSub32: write_line("fsub32"); ip++; break;
            case OpCode::FSub64: write_line("fsub64"); ip++; break;
            case OpCode::FMul32: write_line("fmul32"); ip++; break;
            case OpCode::FMul64: write_line("fmul64"); ip++; break;
            case OpCode::FDiv32: write_line("fdiv32"); ip++; break;
            case OpCode::FDiv64: write_line("fdiv64"); ip++; break;
            case OpCode::Or: write_line("or");         ip++; break;
            case OpCode::And: write_line("and");       ip++; break;
            case OpCode::Not: write_line("not");       ip++; break;
            case OpCode::Alloca: write_line("alloca"); ip++; break;
            case OpCode::PtrOff: write_line("ptroff"); ip++; break;
            case OpCode::FNeg32: write_line("fneg32"); ip++; break;
            case OpCode::FNeg64: write_line("fneg64"); ip++; break;
            case OpCode::Jump: write_line("jump");     ip++; break;
            case OpCode::JumpIfFalse: write_line("jump_if_false"); ip++; break;
            case OpCode::Ret: write_line("ret"); ip++; break;
            case OpCode::RetVoid: write_line("ret void"); ip++; break;
            case OpCode::Constant8: write_line(std::format("const8 {:d}", *++ip)); ip++; break;
            case OpCode::Constant16:
            {
                // there may be padding bytes because it must be 2 byte aligned
                auto offset = ++ip - base;
                ip += (2 - offset % 2) % 2;
                write_line(std::format("const16 {}", *reinterpret_cast<const uint16_t*>(ip)));
                ip += 2; break;
            }
            case OpCode::Constant32:
            {
                // there may be padding bytes because it must be 2 byte aligned
                auto offset = ++ip - base;
                ip += (4 - offset % 4) % 4;
                write_line(std::format("const32 {}", *reinterpret_cast<const uint32_t*>(ip)));
                ip += 4; break;
            }
            case OpCode::Constant64:
            {
                // there may be padding bytes because it must be 2 byte aligned
                auto offset = ++ip - base;
                ip += (8 - offset % 8) % 8;
                write_line(std::format("const64 {}", *reinterpret_cast<const uint64_t*>(ip)));
                ip += 8; break;
            }
            case OpCode::Constant64FromU8: write_line(std::format("const64 from u8 {}", static_cast<uint8_t>(*++ip))); ip++; break;
            case OpCode::Constant64FromI8: write_line(std::format("const64 from i8 {}",*reinterpret_cast<const int8_t*>(++ip))); ip++; break;
            case OpCode::Constant64FromI16:
            {
                auto offset =
                    reinterpret_cast<const uint8_t*>(++ip) -
                    reinterpret_cast<const uint8_t*>(base);
                ip += (2 - offset % 2) % 2;
                write_line(std::format("const64 from i16 {}", *reinterpret_cast<const uint16_t*>(ip)));
                ip += 2; break;
            }
            case OpCode::Constant64FromU16:
            {
                auto offset =
                    reinterpret_cast<const uint8_t*>(++ip) -
                    reinterpret_cast<const uint8_t*>(base);
                ip += (2 - offset % 2) % 2;
                write_line(std::format("const64 from u16 {}", *reinterpret_cast<const int16_t*>(ip)));
                ip += 2; break;
            }
            case OpCode::Constant64FromU32:
            {
                // there may be padding bytes because it must be 4 byte aligned
                auto offset =
                    reinterpret_cast<const uint8_t*>(++ip) -
                    reinterpret_cast<const uint8_t*>(base);
                ip += (4 - offset % 4) % 4;
                write_line(std::format("const64 from u32 {}", *reinterpret_cast<const uint32_t*>(ip)));
                ip += 4; break;
            }
            case OpCode::Constant64FromI32:
            {
                // there may be padding bytes because it must be 4 byte aligned
                auto offset =
                    reinterpret_cast<const uint8_t*>(++ip) -
                    reinterpret_cast<const uint8_t*>(base);
                ip += (4 - offset % 4) % 4;
                write_line(std::format("const64 from i32 {}", *reinterpret_cast<const int32_t*>(ip)));
                ip += 4; break;
            }
            case OpCode::ExternalIntrinsic: write_line(std::format("extern {:d}", *++ip)); ip++; break;
            case OpCode::ConstantPtr:
            {
                // there may be padding bytes because it must be 2 byte aligned
                constexpr auto ptr_size = sizeof(void*);
                auto offset = ++ip - base;
                ip += (ptr_size - offset % ptr_size) % ptr_size;

                auto ptr = *reinterpret_cast<void* const*>(ip);
                auto fn_name = vm->name_of(ptr);
                if (!fn_name.empty()) fn_name = "[[" + fn_name + "]]";
                else if (vm->is_registered_string(reinterpret_cast<const char*>(ptr))) {
                    fn_name = std::string("\"") + reinterpret_cast<const char*>(ptr) + "\"";
                    std::string::size_type pos = 0;
                    while ((pos = fn_name.find("\n", pos)) != std::string::npos) {
                        fn_name.replace(pos, 1, "\\n");
                        pos += 2;
                    }
                }
                write_line(std::format("const ptr {} {}", ptr, fn_name));
                ip += ptr_size; break;
            }
            case OpCode::RegObj: write_line("reg_obj"); ip++; break;
            case OpCode::Panic: write_line("panic"); ip++; break;
            case OpCode::CheckReg: write_line("check_reg"); ip++; break;
            case OpCode::PopReg: write_line("pop_reg"); ip++; break;
            case OpCode::Pop: write_line("pop"); ip++; break;
            case OpCode::Checkpoint: write_line(std::format("checkpoint {:d}", *++ip)); ip++; break;
            case OpCode::Dup: write_line("dup"); ip++; break;
            case OpCode::Switch: write_line("switch"); ip++; break;
            case OpCode::TopConsume: write_line("top_consime"); ip++; break;
            case OpCode::CmpEq: write_line(std::format("cmp_eq {:d}", *++ip)); ip++; break;
            case OpCode::CmpNe: write_line(std::format("cmp_ne {:d}", *++ip)); ip++; break;
            case OpCode::UCmpGt: write_line(std::format("ucmp_gt {:d}", *++ip)); ip++; break;
            case OpCode::UCmpGe: write_line(std::format("ucmp_ge {:d}", *++ip)); ip++; break;
            case OpCode::UCmpLt: write_line(std::format("ucmp_lt {:d}", *++ip)); ip++; break;
            case OpCode::UCmpLe: write_line(std::format("ucmp_le {:d}", *++ip)); ip++; break;
            case OpCode::ICmpGt: write_line(std::format("icmp_gt {:d}", *++ip)); ip++; break;
            case OpCode::ICmpGe: write_line(std::format("icmp_ge {:d}", *++ip)); ip++; break;
            case OpCode::ICmpLt: write_line(std::format("icmp_lt {:d}", *++ip)); ip++; break;
            case OpCode::ICmpLe: write_line(std::format("icmp_le {:d}", *++ip)); ip++; break;
            case OpCode::FCmpEq: write_line(std::format("fcmp_eq {:d}", *++ip)); ip++; break;
            case OpCode::FCmpNe: write_line(std::format("fcmp_ne {:d}", *++ip)); ip++; break;
            case OpCode::FCmpGt: write_line(std::format("fcmp_gt {:d}", *++ip)); ip++; break;
            case OpCode::FCmpGe: write_line(std::format("fcmp_ge {:d}", *++ip)); ip++; break;
            case OpCode::FCmpLt: write_line(std::format("fcmp_lt {:d}", *++ip)); ip++; break;
            case OpCode::FCmpLe: write_line(std::format("fcmp_le {:d}", *++ip)); ip++; break;
            case OpCode::Shl: write_line(std::format("shl {:d}", *++ip)); ip++; break;
            case OpCode::Shr: write_line(std::format("shr {:d}", *++ip)); ip++; break;
            case OpCode::BitAnd: write_line(std::format("bitand {:d}", *++ip)); ip++; break;
            case OpCode::BitOr: write_line(std::format("bitor {:d}", *++ip)); ip++; break;
            case OpCode::BitXor: write_line(std::format("bitxor {:d}", *++ip)); ip++; break;
            case OpCode::StackAddr: write_line(std::format("stackaddr {:d}", *++ip)); ip++; break;
            case OpCode::RevStackAddr: write_line(std::format("stackaddr rev {:d}", *++ip)); ip++; break;
            case OpCode::StackCheckpoint: write_line(std::format("stackaddr checkpoint {:d}", *++ip)); break;
            case OpCode::PtrOffConst: write_line(std::format("ptroff const {:d}", *++ip)); ip++; break;
            case OpCode::AllocaConst: write_line(std::format("alloca const {:d}", *++ip)); ip++; break;
            case OpCode::Call: write_line(std::format("call {:d}", *++ip)); ip++; break;
            case OpCode::NativeCall: write_line(std::format("native call {:d}", *++ip)); ip++; break;
            case OpCode::Malloc: write_line("malloc"); ip++; break;
            case OpCode::Free: write_line("free"); ip++; break;
            case OpCode::MemCpy: write_line("memcpy"); ip++; break;
            case OpCode::Load: 
                write_line(std::format("load {}", load_str(*++ip))); ip++; break;
            case OpCode::Store: 
                write_line(std::format("store {}", load_str(*++ip))); ip++; break;
            case OpCode::UConv: 
            {
                ip += 2;
                write_line(std::format("uconv  from {:d} to {:d}", *(ip - 1), *ip)); ip++; break;
            }
            case OpCode::SConv:
            {
                ip += 2;
                write_line(std::format("sconv  from {:d} to {:d}", *(ip - 1), *ip)); ip++; break;
            }
            case OpCode::FpConv:
            {
                ip += 2;
                write_line(std::format("fpconv  from {:d} to {:d}", *(ip - 1), *ip)); ip++; break;
            }
            case OpCode::FpToSi: write_line("unimplemented"); ip++; break;
            case OpCode::FpToUi: write_line("unimplemented"); ip++; break;
            case OpCode::UiToFp: write_line("unimplemented"); ip++; break;
            case OpCode::SiToFp: write_line("unimplemented"); ip++; break;
            default: write_line(std::format("error({:d})", *ip)); ip++; break;
            }
        }
        return asm_code;
    }
}