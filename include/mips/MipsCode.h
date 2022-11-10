#ifndef MIPS_CODE_H
#define MIPS_CODE_H
#pragma once

#include <cassert>
#include <iomanip>
#include <iostream>
#include <set>
#include <vector>

#include <ir/IR.h>
#include "MipsCodeTypeEnum.h"
#include "MipsRegEnum.h"

struct MipsFunc;
struct MipsBasicBlock;
struct MipsInst;
struct MipsOperand;

enum class MipsCond { Any,
                      Eq,
                      Ne,
                      Ge,
                      Gt,
                      Le,
                      Lt };

inline MipsCond oppositeCond(MipsCond c) {
    constexpr static MipsCond OPPOSITE[] = {MipsCond::Any, MipsCond::Ne, MipsCond::Eq, MipsCond::Lt,
                                            MipsCond::Le, MipsCond::Gt, MipsCond::Ge};
    return OPPOSITE[static_cast<int>(c)];
}

struct MipsShift {
    enum {
        None, // no shifting
        Sra,  // arithmetic right
        Sll,  // logic left
        Srl,  // logic right
    } type;
    int shift;

    MipsShift() {
        shift = 0;
        type = None;
    }

    bool isNone() const { return type == None; }

    explicit operator std::string() const {
        const char* name = "";
        switch (type) {
        case MipsShift::Sra:
            name = "sra";
            break;
        case MipsShift::Sll:
            name = "sll";
            break;
        case MipsShift::Srl:
            name = "srl";
            break;
        default: break;
        }
        return std::string(name) + std::to_string(shift);
    }
};

inline std::ostream& operator<<(std::ostream& os, const MipsShift& shift) {
    os << std::string(shift);
    return os;
}

inline std::ostream& operator<<(std::ostream& os, const MipsCond& cond) {
    if (cond == MipsCond::Eq) {
        os << "eq";
    } else if (cond == MipsCond::Ne) {
        os << "ne";
    } else if (cond == MipsCond::Any) {
        os << "";
    } else if (cond == MipsCond::Gt) {
        os << "gt";
    } else if (cond == MipsCond::Ge) {
        os << "ge";
    } else if (cond == MipsCond::Lt) {
        os << "lt";
    } else if (cond == MipsCond::Le) {
        os << "le";
    }
    return os;
}

struct MipsProgram {
    std::vector<MipsFunc> func;
    friend std::ostream& operator<<(std::ostream& os, const MipsProgram& dt);
};

std::ostream& operator<<(std::ostream& os, const MipsProgram& dt);

struct MipsFunc {
    std::vector<MipsBasicBlock> bb;
    IrFunc* func;
    // number of virtual registers allocated
    int virtual_max = 0;
    // size of stack allocated for local alloca and spilled registers
    int stack_size = 0;
    // set of callee saved registers used
    std::set<MipsReg> used_callee_saved_regs;
    // whether lr is allocated
    bool use_lr = false;
    // offset += stack_size + saved_regs * 4;
    std::vector<MipsInst*> sp_arg_fixup;
};

struct MipsBasicBlock {
    BasicBlock* bb;
    std::list<std::unique_ptr<MipsInst>> insts;
    // predecessor and successor
    std::vector<MipsBasicBlock*> pred;
    std::array<MipsBasicBlock*, 2> succ;
    // branch is translated into multiple instructions
    // points to the first one
    MipsInst* control_transfer_inst = nullptr;
    // liveness analysis
    // maybe we should use bitset when performance is bad
    std::set<MipsOperand> liveuse;
    std::set<MipsOperand> def;
    std::set<MipsOperand> livein;
    std::set<MipsOperand> liveout;
};

struct MipsOperand {
    enum class State {
        PreColored,
        Allocated,
        Virtual,
        Immediate,
    } state;
    int value;

    inline static MipsOperand R(MipsReg r) {
        auto n = (int)r;
        assert(n >= int(MipsReg::zero) && n <= int(MipsReg::t9));
        return MipsOperand{State::PreColored, n};
    }

    inline static MipsOperand V(int n) { return MipsOperand{State::Virtual, n}; }

    inline static MipsOperand I(int imm) { return MipsOperand{State::Immediate, imm}; }

    // both are PreColored or Allocated, and has the same value
    bool is_equiv(const MipsOperand& other) const {
        return (state == State::PreColored || state == State::Allocated) && (other.state == State::PreColored || other.state == State::Allocated) && value == other.value;
    }

    bool operator<(const MipsOperand& other) const {
        if (state != other.state) {
            return state < other.state;
        } else {
            return value < other.value;
        }
    }

    bool operator==(const MipsOperand& other) const { return state == other.state && value == other.value; }

    bool operator!=(const MipsOperand& other) const { return state != other.state || value != other.value; }

    bool isVirtual() const { return state == State::Virtual; }
    bool isImm() const { return state == State::Immediate; }
    bool isPrecolored() const { return state == State::PreColored; }
    bool isReg() const { return state == State::PreColored || state == State::Allocated || state == State::Virtual; }
    bool needsColor() const { return state == State::Virtual || state == State::PreColored; }

    explicit operator std::string() const {
        char prefix = '?';
        switch (this->state) {
        case State::PreColored:
        case State::Allocated:
            prefix = 'r';
            break;
        case State::Virtual:
            prefix = 'v';
            break;
        case State::Immediate:
            prefix = '#';
            break;
        }
        return prefix + std::to_string(this->value);
    }

    friend std::ostream& operator<<(std::ostream& os, const MipsOperand& op) {
        os << std::string(op);
        return os;
    }
};

namespace std {
template <>
struct hash<MipsOperand> {
    std::size_t operator()(MipsOperand const& m) const noexcept {
        // state (2), value (14)
        return ((((size_t)m.state) << 14u) | (int)m.value) & 0xFFFFu;
    }
};

template <>
struct hash<std::pair<MipsOperand, MipsOperand>> {
    std::size_t operator()(std::pair<MipsOperand, MipsOperand> const& m) const noexcept {
        // hash(second), hash(first)
        hash<MipsOperand> hash_func;
        return (hash_func(m.second) << 16u) | hash_func(m.first);
    }
};
} // namespace std

class MipsInst {
public:
    MipsInst(MipsCodeType type) :
        m_type(type) {}

protected:
    MipsBasicBlock* m_bb;
    MipsCodeType m_type;
};

class MipsBinary : public MipsInst {
public:
    MipsBinary(MipsCodeType type) :
        MipsInst(type) {}

    bool isIdentity() {
        switch (m_type) {
        case MipsCodeType::Add:
        case MipsCodeType::Sub:
            return m_dst.is_equiv(m_lhs) && m_rhs == MipsOperand::I(0) && m_shift.type == MipsShift::None;
        default:
            return false;
        }
    }

private:
    // Add, Sub, Rsb, Mul, Div, Mod, Lt, Le, Ge, Gt, Eq, Ne, And, Or
    MipsOperand m_dst;
    MipsOperand m_lhs;
    MipsOperand m_rhs;
    MipsShift m_shift;
};

class MipsMove : public MipsInst {
    friend class MipsMoveCompare;

public:
    MipsMove() :
        MipsInst(MipsCodeType::Move), m_cond(MipsCond::Any) {}
    bool isSimple() { return m_cond == MipsCond::Any && m_shift.type == MipsShift::None; }

private:
    MipsCond m_cond;
    MipsOperand m_dst;
    MipsOperand m_rhs;
    MipsShift m_shift;
};

struct MipsMoveCompare {
    bool operator()(MipsMove* const& lhs, const MipsMove* const& rhs) const {
        if (lhs->m_cond != rhs->m_cond) return lhs->m_cond < rhs->m_cond;
        if (lhs->m_dst != rhs->m_dst) return lhs->m_dst < rhs->m_dst;
        if (lhs->m_rhs != rhs->m_rhs) return lhs->m_rhs < rhs->m_rhs;
        return false;
    }
};

class MipsBranch : public MipsInst {
public:
    MipsBranch() :
        MipsInst(MipsCodeType::Branch) {}

private:
    MipsCond m_cond;
    MipsBasicBlock* m_target;
};

class MipsJump : public MipsInst {
public:
    MipsJump(MipsBasicBlock* target) :
        MipsInst(MipsCodeType::Jump), m_target(target) {}

private:
    MipsBasicBlock* m_target;
};

class MipsReturn : public MipsInst {
public:
    MipsReturn() :
        MipsInst(MipsCodeType::Return) {}
};

class MipsAccess : public MipsInst {
public:
    enum class Mode {
        Offset,
        Prefix,
        Postfix,
    };
    MipsAccess(MipsCodeType type) :
        MipsInst(type), m_cond(MipsCond::Any) {}

private:
    MipsOperand m_addr;
    MipsOperand m_offset;
    int m_shift;
    MipsCond m_cond;
    Mode m_mode;
};

class MipsLoad : public MipsAccess {
public:
    MipsLoad() :
        MipsAccess(MipsCodeType::Load) {}

private:
    MipsOperand m_dst;
};

class MipsStore : public MipsAccess {
public:
    explicit MipsStore() :
        MipsAccess(MipsCodeType::Store) {}

private:
    MipsOperand m_data;
};

class MipsCompare : public MipsInst {
public:
    explicit MipsCompare() :
        MipsInst(MipsCodeType::Compare) {}

private:
    MipsOperand m_lhs;
    MipsOperand m_rhs;
};

class MipsCall : public MipsInst {
public:
    explicit MipsCall() :
        MipsInst(MipsCodeType::Call) {}

private:
    FuncItem* m_func;
};

class MipsGlobal : public MipsInst {
public:
    MipsGlobal(SymbolTableItem* sym) :
        MipsInst(MipsCodeType::Global), m_sym(sym) {}

private:
    MipsOperand m_dst;
    SymbolTableItem* m_sym;
};

class MipsComment : public MipsInst {
public:
    MipsComment(const std::string& content) :
        MipsInst(MipsCodeType::Comment), m_content(content) {}

private:
    std::string m_content;
};

#endif