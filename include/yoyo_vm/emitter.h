#pragma once
#include "common.h"
#include <set>
#include <unordered_map>
#include <string>
#include <vector>

#include "instructions.h"
#include "writer.h"
#include "vm.h"
namespace Yvm
{
    enum class Type : uint8_t
    {
        i8 = 0, i16 = 1, i32 = 2, i64 = 3,
        u8 = 4, u16 = 5, u32 = 6, u64 = 7,
        f32 = 8, f64 = 9, ptr = 10,
    };
    struct YVM_API ConstString {
        const char* data;
        size_t size;
    };
    /// Utility class to write bytecode
    /// It handles the weirdness of constant alignment and jump addresses
    class YVM_API Emitter
    {
        Writer writer;
        Writer alloca_writer;
        std::unordered_map<std::string, uint64_t> jump_addrs;
        std::vector<std::pair<size_t, std::string>> function_addrs;
        std::unordered_map<size_t, std::string> unresolved_jumps;
        std::set<std::string> label_reservations;
        size_t last_alloc = 0;
        size_t last_checkpoint = 0;
        std::string unique_name_from(const std::string& name) const;
        bool unify_alloca;
    public:
        Emitter(bool unify_alloca = true);
        OpCode last_inst;
        /// Emit a single byte instruction (eg add32)
        void write_1b_inst(OpCode code);
        /// Emit a 2 byte instruction (eg stackaddr)
        void write_2b_inst(OpCode code, uint8_t arg);
        /// Emit a 2 byte instruction but with a specified type
        /// Useful for load/store
        void write_2b_inst(OpCode code, Type arg);
        /// Emit a 3 byte instruction like conversion operations
        void write_3b_inst(OpCode code, uint8_t arg1, uint8_t arg2);
        /// Mark this point as a label to be used by jumps
        /// The name may already be taken, in that case you get a new name.
        /// The name used is returned and can be used for jumps
        /// You can also use @link unq_label_name to get a label name and reserve it
        std::string create_label(const std::string& name);
        /// reserved a unique label name to be used in future jumps
        /// you can use this label to jump even before calling @link create_label.
        /// If you do, you must call @link resolve_jumps to substitute the correct address
        std::string unq_label_name(const std::string& name);
        /// Create a constant jump to a specified label
        /// Jump is also a 1 byte instruction if you want to use dynamic offsets
        void create_jump(OpCode code, const std::string& label_name);
        /// write the address of a function as a constant
        void write_fn_addr(const std::string& fn_name);
        /// write a pointer to a string constant
        void write_const_string(const ConstString& str);
        size_t last_alloc_addr();
        void add_function_params(size_t n);
        /// adds a string to the binary
        ConstString create_const_string(std::string text, VM* code);
        /// end the function (it ensures there's a ret)
        /// and adds it to a module
        void close_function(Module* mod, const std::string& name);
        /// Write alloca_const or alloca based on size
        /// the return value is the stack addr is using unified alloca's
        size_t write_alloca(uint32_t size);
        void write_ptr_off(uint32_t size);
        /// Write a constant instruction
        /// @tparam T the type, it must be an integral or floating point type from 1-8 bytes
        template<typename T>
        void write_const(T value);
        /// Call this after you're done emitting code to replace jump labels
        /// that were undefined at the time with their appropriate address
        /// It's just a safe bet to call this everytime you're done even if you don't use jumps
        void resolve_jumps();
        /// Retrieve the code generated so far
        const std::vector<uint64_t>& get_code() const&;
        /// Move out the code generated so far.
        ///
        /// Prefer @code std::move(emitter).get_code() @endcode to
        /// @code std::move(emitter.get_code()) @endcode, the former leaves
        /// @c emitter in a valid state to be used again
        std::vector<uint64_t> get_code() &&;
        /// Get the name of the last function name used in @c write_fn_addr
        /// The reference is invalidated once the @c write_fn_addr is called again
        std::string& get_last_inserted_function();
        /// Insert a stack checkpoint
        /// and return the index of the inserted checkpoint
        size_t checkpoint();
    };

    template <typename T>
    void Emitter::write_const(T value)
    {
        constexpr auto size = sizeof(T);
        static_assert(size == 1 || size == 2 || size == 4 || size == 8, "Invalid constant type");
        if constexpr (std::is_pointer<T>::value) write_1b_inst(OpCode::ConstantPtr);
        else if constexpr (std::is_same_v<uint64_t, T>)
        {
            // utilize optimized constants
            if (value <= std::numeric_limits<uint8_t>::max()) {
                write_1b_inst(OpCode::Constant64FromU8);
                writer.write_n<uint8_t>(value);
            }
            else if (value <= std::numeric_limits<uint16_t>::max()) {
                write_1b_inst(OpCode::Constant64FromU16);
                writer.write_n<uint16_t>(value);
            }
            else if (value <= std::numeric_limits<uint32_t>::max()) {
                write_1b_inst(OpCode::Constant64FromU32);
                writer.write_n<uint32_t>(value);
            }
            else {
                write_1b_inst(OpCode::Constant64);
                writer.write_n<T>(value);
            }
            return;
        }
        else if constexpr (std::is_same_v<int64_t, T>)
        {
            if (value <= std::numeric_limits<int8_t>::max() && value >= std::numeric_limits<int8_t>::min()) {
                write_1b_inst(OpCode::Constant64FromU8);
                writer.write_n<int8_t>(value);
            }
            else if (value <= std::numeric_limits<int16_t>::max() && value >= std::numeric_limits<int16_t>::min()) {
                write_1b_inst(OpCode::Constant64FromU16);
                writer.write_n<int16_t>(value);
            }
            else if (value <= std::numeric_limits<int32_t>::max() && value >= std::numeric_limits<int32_t>::min()) {
                write_1b_inst(OpCode::Constant64FromU32);
                writer.write_n<int32_t>(value);
            }
            else {
                write_1b_inst(OpCode::Constant64);
                writer.write_n<T>(value);
            }
            return;
        }
        else if constexpr (std::is_same_v<float, T>) {
            write_1b_inst(OpCode::ConstantF32);
        }
        else if constexpr (std::is_same_v<double, T>) {
            write_1b_inst(OpCode::ConstantF64);
        }
        else if constexpr (size == 1) write_1b_inst(OpCode::Constant8);
        else if constexpr (size == 2) write_1b_inst(OpCode::Constant16);
        else if constexpr (size == 4) write_1b_inst(OpCode::Constant32);
        else static_assert(false);
        writer.write_n<T>(value);
    }
}
