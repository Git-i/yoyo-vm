#pragma once
#include <cstdint>


/// Type Bytes
struct Type
{
    constexpr uint8_t
};

enum class OpCode : uint8_t {
    //-------1 byte Operations-----------------------
    // Integer operations, hopefully we're in two's complement
    Add8, Add16, Add32, Add64,
    Sub8, Sub16, Sub32, Sub64,
    Mul8, Mul16, Mul32, Mul64,
    // Division and Module operations
    IDiv8, IDiv16, IDiv32, IDiv64,
    UDiv8, UDiv16, UDiv32, UDiv64,
    IRem8, IRem16, IRem32, IRem64,
    URem8, URem16, URem32, URem64,
    // Arithmetic operations for floats
    FAdd32, FAdd64,
    FSub32, FSub64,
    FMul32, FMul64,
    FDiv32, FDiv64,
    // Comparison Operations
    CmpEq, CmpNe,
    UCmpGt, UCmpGe, UCmpLt, UCmpLe,
    ICmpGt, ICmpGe, ICmpLt, ICmpLe,
    // Negation Operations

    /// jump to an offset specified on the stack
    /// the offset must be a 64-byte unsigned integer
    /// the offset is specified in terms of the current function
    Jump,
    //---------------------------------------------------
    /// pop the stack top, if its non-zero this is equivalent to @c Jump
    /// else it pops the stack to clear the jump address
    JumpIf,
    /// Return the top of the stack and go back to the calling function
    Ret,
    /// Equivalent to @code Contant8 0; Ret;@endcode
    /// but takes one byte instead of 3
    RetVoid,
    Constant8, Constant16, Constant32, Constant64,
    //------2 byte Operations-----------------------------
    //push a value from a constant spot in the stack onto the stack top
    StackAddr,
    /// pop the function pointer from the top of the stack
    /// and use the next n stack items as parameters of the function
    /// n is specified as the second byte of the operation
    /// it pops n values from the stack and pushes the result of the call
    Call
};
