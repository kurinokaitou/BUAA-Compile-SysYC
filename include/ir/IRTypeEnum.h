#ifndef IR_TYPE_ENUM_H
#define IR_TYPE_ENUM_H
enum class IRType {
#include "IR.inc"
    Branch,
    Jump,
    Return, // Control flow
    GetElementPtr,
    Load,
    Store, // Memory
    Call,
    Alloca,
    Const,
    Global,
    Param,
    Phi,
    Print
};

#endif