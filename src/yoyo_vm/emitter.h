#pragma once
#include <set>
#include <unordered_map>
#include <string>
#include <vector>

#include "instructions.h"
#include "writer.h"
namespace Yvm
{
    enum class Type : uint8_t
    {
        i8 = 0, i16 = 1, i32 = 2, i64 = 3,
        u8 = 4, u16 = 5, u32 = 6, u64 = 7,
        f32 = 8, f64 = 9, ptr = 10,
    };
    class Emitter
    {
        Writer writer;
        std::unordered_map<std::string, uint64_t> jump_addrs;
        std::unordered_map<size_t, std::string> unresolved_jumps;
        std::set<std::string> label_reservations;

        std::string unique_name_from(const std::string& name) const;
    public:
                 /// Emit a single byte instruction (eg add32)
        void write_1b_inst(OpCode code);
        /// Emit a 2 byte instruction (eg load, store)
        void write_2b_inst(OpCode code, uint8_t arg);
        void write_2b_inst(OpCode code, Type arg);
        /// Emit a 3 byte instruction like conversion operations
        void write_3b_inst(OpCode code, uint8_t arg1, uint8_t arg2);
        /// Mark this point as a label to be used by jumps
        /// The name may already be taken, in that case you get a new name
        /// The name used is returned and can be used for jumps
        /// You can also use @c unq_label_name to get a label name and reserve it
        std::string create_label(const std::string& name);
        std::string unq_label_name(const std::string& name);
        void create_jump(OpCode code, const std::string& label_name);
        /// Write alloca_const or alloca based on size
        void write_alloca(uint32_t size);
        template<typename T>
        void write_const(T value);
        void resolve_jumps();

        const std::vector<uint64_t>& get_code() const&;
        std::vector<uint64_t> get_code() &&;
    };

    template <typename T>
    void Emitter::write_const(T value)
    {
        constexpr auto size = sizeof(T);
        static_assert(size == 1 || size == 2 || size == 4 || size == 8, "Invalid constant type");
        if constexpr (size == 1) write_1b_inst(OpCode::Constant8);
        else if constexpr (size == 2) write_1b_inst(OpCode::Constant16);
        else if constexpr (size == 4) write_1b_inst(OpCode::Constant32);
        else if constexpr (size == 8) write_1b_inst(OpCode::Constant64);
        writer.write_n<T>(value);
    }
}
