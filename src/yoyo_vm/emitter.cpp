#include "yoyo_vm/emitter.h"

#include <limits>

namespace Yvm
{
    using enum OpCode;
    Emitter::Emitter(bool unify_alloc)
    {
        unify_alloca = unify_alloc;
        last_inst = Nop;
        if (unify_alloc) {
            create_jump(Jump, unq_label_name("alloca_region")); // no need to store the name, because its guaranteed not to collide
            create_label("entry");
        }
    }
    size_t Emitter::write_alloca(uint32_t size)
    {
        if (size > std::numeric_limits<uint8_t>::max())
        {
            if (unify_alloca) {
                alloca_writer.write_n(size);
                alloca_writer.write_opcode(Alloca);
                // ensure the allocation is at stack top
                write_2b_inst(StackAddr, last_alloc);
                return last_alloc++;
            }
            else {
                write_const(size); write_1b_inst(Alloca);
                return 0;
            }
        }
        else
        {
            if (unify_alloca) {
                alloca_writer.write_opcode(AllocaConst);
                alloca_writer.write_byte(size);
                // ensure the allocation is at stack top
                write_2b_inst(StackAddr, last_alloc);
                return last_alloc++;
            }
            else write_2b_inst(AllocaConst, size);
            return 0;
        }
    }

    void Emitter::resolve_jumps()
    {
        for (auto& [code, label] : unresolved_jumps)
        {
            writer.data[code] = jump_addrs[label];
        }
    }
    void Emitter::write_fn_addr(const std::string& fn_name) {
        write_const<void*>(0);
        auto addr = writer.data.size() - 1;
        function_addrs.emplace_back(addr, fn_name);
    }
    void Emitter::close_function(Module* mod, const std::string& name) {
        if (last_inst == OpCode::Ret || last_inst == OpCode::RetVoid) return;
        write_1b_inst(OpCode::RetVoid);
        if (unify_alloca) {
            alloca_writer.write_opcode(Constant64);
            alloca_writer.write_n(jump_addrs["entry"]);
            alloca_writer.write_opcode(Jump);
            jump_addrs["alloca_region"] = writer.data.size() * 8;
            writer.data.insert(writer.data.end(), alloca_writer.data.begin(), alloca_writer.data.end());
        }
        resolve_jumps();
        mod->code[name] = std::move(writer.data);
        auto& this_fn = mod->code[name];

        for (auto& [addr, fn_name] : function_addrs) {
            mod->unresolved_externals[reinterpret_cast<void**>(&this_fn[addr])] = fn_name;
        }

        writer.byte_off = 0;
        writer.data.clear();
        label_reservations.clear();
        jump_addrs.clear();
        alloca_writer.byte_off = 0;
        alloca_writer.data.clear();
    }
    void Emitter::write_ptr_off(uint32_t off) {
        // we can statically skip the instruction
        if (off == 0) return;
        if (off > std::numeric_limits<uint8_t>::max())
        {
            write_const<uint32_t>(off);
            write_1b_inst(PtrOff);
        }
        else
        {
            write_2b_inst(PtrOffConst, off);
        }
    }
    void Emitter::write_const_string(const ConstString& str)
    {
        write_const(str.data);
    }
    ConstString Emitter::create_const_string(std::string text, VM* code)
    {
        auto size = text.size();
        return ConstString{ code->add_string(std::move(text)), size };
    }
    const std::vector<uint64_t>& Emitter::get_code() const&
    {
        return writer.data;
    }
    std::vector<uint64_t> Emitter::get_code() &&
    {
        auto vec = std::move(writer.data);
        writer.data.clear();
        return vec;
    }
    std::string Emitter::unique_name_from(const std::string& name) const
    {
        if (!jump_addrs.contains(name) && !label_reservations.contains(name))
            return name;
        size_t num = 1;
        while (true)
        {
            auto new_name = name + "." + std::to_string(num);
            if (!jump_addrs.contains(new_name) && !label_reservations.contains(new_name))
                return new_name;
            num++;
        }
    }

    void Emitter::write_1b_inst(OpCode code)
    {
        writer.write_opcode(code);
    }

    void Emitter::write_2b_inst(OpCode code, uint8_t arg)
    {
        writer.write_opcode(code);
        writer.write_byte(arg);
    }
    void Emitter::write_3b_inst(OpCode code, uint8_t arg1, uint8_t arg2)
    {
        writer.write_opcode(code);
        writer.write_byte(arg1);
        writer.write_byte(arg2);
    }

    std::string Emitter::create_label(const std::string& name)
    {
        if (label_reservations.contains(name))
        {
            label_reservations.erase(name);
            jump_addrs[name] = writer.byte_off;
            return name;
        }

        auto new_name = unique_name_from(name);
        jump_addrs[new_name] = writer.byte_off;
        return new_name;
    }

    void Emitter::write_2b_inst(OpCode code, Type arg)
    {
        writer.write_opcode(code);
        writer.write_byte(static_cast<uint8_t>(arg));
    }

    std::string Emitter::unq_label_name(const std::string& name)
    {
        std::string new_name = unique_name_from(name);
        label_reservations.insert(new_name);
        return new_name;
    }

    void Emitter::create_jump(OpCode code, const std::string& label_name)
    {
        writer.write_opcode(Constant64);
        if (jump_addrs.contains(label_name)) writer.write_n(jump_addrs[label_name]);
        else
        {
            writer.write_n<uint64_t>(0);
            auto addr = writer.data.size() - 1;
            unresolved_jumps[addr] = label_name;
        }
        writer.write_opcode(code);
    }
}
