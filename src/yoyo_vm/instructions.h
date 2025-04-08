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

    Alloca,

    /// Offset a pointer by a value from the stack
    PtrOff,
    // Negation Operations
    FNeg32, FNeg64,
    /// jump to an offset specified on the stack
    /// the offset must be a 64-byte unsigned integer
    /// the offset is specified in terms of the current function
    Jump,
    //---------------------------------------------------
    /// pop the stack top, jumps to that address if the stack top is non zero
    JumpIfFalse,
    /// Return the top of the stack and go back to the calling function
    Ret,
    /// Equivalent to @code Contant8 0; Ret;@endcode
    /// but takes one byte instead of 3
    RetVoid,
    Constant8, Constant16, Constant32, Constant64,

    Panic,
    //------2 byte Operations-----------------------------
    // Comparison Operations
    CmpEq, CmpNe,
    UCmpGt, UCmpGe, UCmpLt, UCmpLe,
    ICmpGt, ICmpGe, ICmpLt, ICmpLe,

    FCmpEq, FCmpNe, FCmpGt, FCmpGe, FCmpLt, FCmpLe,
    // Bit operations are not too common so we make them two bytes
    Shl, Shr, BitAnd, BitOr, BitXor,
    //push a value from a constant spot in the stack onto the stack top
    StackAddr,
    /// Offset a pointer by a value specified in the next byte
    PtrOffConst,
    AllocaConst,
    /// pop the function pointer from the top of the stack
    /// and use the next n stack items as parameters of the function
    /// n is specified as the second byte of the operation
    /// it pops n values from the stack and pushes the result of the call
    Call,
    // for load and store the next byte specifies the type, it can be one of:
    // 0 -> i8, 1 -> i16, 2 -> i32, 3 -> i64, 4 -> u8, 5 -> u16, 6 -> u32, 7 -> u64
    // 8 -> f32, 9 -> f64, 10 -> ptr
    Load,
    Store,
    //------3 byte Operations-------------------------------
    // the next byte specifies the size of the source integer
    // and the third byte specifies the size of the destination integer
    UConv, SConv,
    FpConv,
    FpToSi, FpToUi, UiToFp, SiToFp,
};
