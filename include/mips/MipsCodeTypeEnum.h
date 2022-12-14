#ifndef MIPS_CODE_TYPE_ENUM_H
#define MIPS_CODE_TYPE_ENUM_H
enum class MipsCodeType {
#include <ir/Op.inc>
    // Binary
    Move,
    Branch,
    Shift,
    Jump,
    Return, // Control flow
    Load,
    Store, // Memory
    Compare,
    Call,
    Global,
    Print, // for printing
};
#endif