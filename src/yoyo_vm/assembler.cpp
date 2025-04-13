#include "yoyo_vm/assembler.h"

#include <charconv>
#include <iostream>
#include <ranges>
#include <unordered_map>

#include "instructions.h"
#include "yoyo_vm/writer.h"
namespace Yvm
{

    static inline void lstrip(std::string_view& arg)
    {
        arg.remove_prefix(std::min(arg.find_first_not_of(" \n\t\r"), arg.size()));
    }
    static inline void rstrip(std::string_view& arg)
    {
        arg.remove_suffix(arg.size() - std::min(arg.find_last_not_of(" ") + 1, arg.size()));
    }
    static inline void strip(std::string_view& arg)
    {
        lstrip(arg); rstrip(arg);
    }
    std::vector<uint64_t> Assembler::assemble(std::string_view code)
    {
        using enum OpCode;
        Writer writer;
        std::unordered_map<std::string_view, OpCode> opcode{
            {"alloc_const", AllocaConst},
            {"const8", Constant8},
            {"s_addr", StackAddr},
            {"store", Store},
            {"load", Load},
            {"icmp_lt", ICmpLt},
            {"icmp_gt", ICmpGt},
            {"add32", Add32},
            {"jump_if_f", JumpIfFalse},
            {"jump", Jump},
            {"ret", Ret}
        };
        for (auto line_ptr : code | std::views::split('\n'))
        {
            std::string_view line{line_ptr.begin(), line_ptr.end()};
            lstrip(line);
            if (line.size() == 0 || line[0] == ';') continue;
            std::string_view instruction{
                line.data(),std::min(line.find_first_of(" \n\t\r"), line.size())
            };
            if (instruction == "alloc_const" ||
                instruction == "s_addr" ||
                instruction.starts_with("icmp") ||
                instruction == "const8")
            {
                if (!opcode.contains(instruction)) return {};
                writer.write_opcode(opcode[instruction]);
                uint8_t val;
                std::string_view num{line.data() + instruction.size(), line.data() + line.size()};
                strip(num);
                auto ec = std::from_chars(num.data(), num.data() + num.size(), val);
                if (ec.ec == std::errc::invalid_argument) return {};
                writer.write_byte(val);
            }
            else if (instruction == "const32")
            {
                writer.write_opcode(Constant32);
                uint32_t val;
                std::string_view num{line.data() + instruction.size(), line.data() + line.size()};
                strip(num);
                auto ec = std::from_chars(num.data(), num.data() + num.size(), val);
                if (ec.ec == std::errc::invalid_argument) return {};
                writer.write_n(val);
            }
            else if (instruction == "const64")
            {
                writer.write_opcode(Constant64);
                uint64_t val;
                std::string_view num{line.data() + instruction.size(), line.data() + line.size()};
                strip(num);
                auto ec = std::from_chars(num.data(), num.data() + num.size(), val);
                if (ec.ec == std::errc::invalid_argument) return {};
                writer.write_n(val);
            }
            else if (instruction == "store" || instruction == "load")
            {
                writer.write_opcode(opcode[instruction]);
                std::string_view type{line.data() + instruction.size(), line.data() + line.size()};
                strip(type);
                if (type == "i8") writer.write_byte(0);
                else if (type == "i16") writer.write_byte(1);
                else if (type == "i32") writer.write_byte(2);
                else if (type == "i64") writer.write_byte(3);
                else if (type == "u8") writer.write_byte(4);
                else if (type == "u16") writer.write_byte(5);
                else if (type == "u32") writer.write_byte(6);
                else if (type == "u64") writer.write_byte(7);
                else if (type == "f32") writer.write_byte(8);
                else if (type == "f64") writer.write_byte(9);
                else if (type == "ptr") writer.write_byte(10);
                else return {};
            }
            else if (instruction.ends_with(':'))
            {
                instruction.remove_suffix(1);
                jump_addrs[instruction] = writer.byte_off;
            }
            else if (instruction == "jump_if_f" || instruction == "jump")
            {
                writer.write_opcode(Constant64);
                writer.write_n(uint64_t{0});
                auto addr = writer.data.size() - 1;
                std::string_view label{line.data() + instruction.size(), line.data() + line.size()};
                strip(label);
                if (jump_addrs.contains(label)) writer.data[addr] = jump_addrs[label];
                else unresolved_jumps[addr] = label;
                writer.write_opcode(opcode[instruction]);
            }
            else if (instruction.starts_with("add")
                || instruction.starts_with("sub")
                || instruction.starts_with("mul")
                || instruction == "ret")
            {
                writer.write_opcode(opcode[instruction]);
            }
            else
                return {};
        }
        //resolve jumps
        for (auto [code, label] : unresolved_jumps)
        {
            writer.data[code] = jump_addrs[label];
        }
        return writer.data;
    }

}