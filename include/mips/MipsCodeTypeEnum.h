#ifndef MIPS_CODE_TYPE_ENUM_H
#define MIPS_CODE_TYPE_ENUM_H
enum class MipsCodeType {
#include <ir/IR.inc>
    // Binary
    Move,
    Branch,
    Jump,
    Return, // Control flow
    Load,
    Store, // Memory
    Compare,
    Call,
    Global,
    Comment, // for printing comments
};
#endif