#pragma once
#include <cstdint>

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
    // Bitwise Operations (no sign)
    Shl8, Shl16, Shl32, Shl64,
    // right shit if logical not arithmetic
    Shr8, Shr16, Shr32, Shr64,
    // I might remove the sized variants and perform on the entire register
    BitAnd8, BitAnd16, BitAnd32, BitAnd64,
    BitOr8, BitOr16, BitOr32, BitOr64,
    BitXor8, BitXor16, BitXor32, BitXor64,

    Alloca,
    Load,
    Store,
    /// Offset a pointer by a value from the stack
    PtrOff,
    // Negation Operations
    FNeg32, FNeg64,
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

    Panic,
    //------2 byte Operations-----------------------------
    //push a value from a constant spot in the stack onto the stack top
    StackAddr,
    /// Offset a pointer by a value specified in the next byte
    PtrOffConst,
    /// pop the function pointer from the top of the stack
    /// and use the next n stack items as parameters of the function
    /// n is specified as the second byte of the operation
    /// it pops n values from the stack and pushes the result of the call
    Call,
    //------3 byte Operations-------------------------------
    // the next byte specifies the size of the source integer
    // and the third byte specifies the size of the destination integer
    Zext, Sext, Trunc,
    FpConv,
    FpToSi, FpToUi, UiToFp, SiToFp,
};
