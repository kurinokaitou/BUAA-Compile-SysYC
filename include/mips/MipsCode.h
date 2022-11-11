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

class MipsFunc;
class MipsBasicBlock;
class MipsInst;
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
        os << "beq";
    } else if (cond == MipsCond::Ne) {
        os << "bne";
    } else if (cond == MipsCond::Any) {
        os << "";
    } else if (cond == MipsCond::Gt) {
        os << "sgt";
    } else if (cond == MipsCond::Ge) {
        os << "sge";
    } else if (cond == MipsCond::Lt) {
        os << "slt";
    } else if (cond == MipsCond::Le) {
        os << "sle";
    }
    return os;
}

class MipsModule {
    friend class IrModule;

public:
    MipsFunc* addFunc(MipsFunc* func) {
        m_funcs.push_back(std::unique_ptr<MipsFunc>(func));
        return m_funcs.back().get();
    }
    void toCode(std::ostream& os);

private:
    std::vector<std::unique_ptr<MipsFunc>> m_funcs;
};

class MipsFunc {
public:
    explicit MipsFunc(IrFunc* irFunc) :
        m_irFunc(irFunc) {}
    MipsBasicBlock* pushBackBasicBlock(MipsBasicBlock* basicBlock) {
        m_basicBlocks.push_back(std::unique_ptr<MipsBasicBlock>(basicBlock));
        return m_basicBlocks.back().get();
    }
    MipsBasicBlock* getFirstBasicBlock() { return m_basicBlocks.front().get(); }
    IrFunc* getIrFunc() { return m_irFunc; }
    std::vector<MipsInst*>& getSpArgFixup() { return m_spArgFixup; }
    void setVirtualMax(int virtualMax) { m_virtualMax = virtualMax; }

private:
    std::vector<std::unique_ptr<MipsBasicBlock>> m_basicBlocks;
    IrFunc* m_irFunc;
    // number of virtual registers allocated
    int m_virtualMax = 0;
    // size of stack allocated for local alloca and spilled registers
    int m_stackSize = 0;
    // set of callee saved registers used
    std::set<MipsReg> m_usedCalleeSavedRegs;
    // whether lr is allocated
    bool m_useLr = false;
    // offset += stack_size + saved_regs * 4;
    std::vector<MipsInst*> m_spArgFixup;
};
class MipsInst {
    friend class MipsBasicBlock;

public:
    MipsInst(MipsCodeType type) :
        m_type(type) {}

protected:
    MipsBasicBlock* m_atBlock;
    MipsCodeType m_type;
};

class MipsBasicBlock {
public:
    explicit MipsBasicBlock(BasicBlock* irbb) :
        m_irBasicBlock(irbb) {}
    std::vector<MipsBasicBlock*>& getPreds() { return m_pred; }
    std::array<MipsBasicBlock*, 2>& getSuccs() { return m_succ; };

    MipsInst* pushBackInst(MipsInst* inst) {
        inst->m_atBlock = this;
        m_insts.push_back(std::unique_ptr<MipsInst>(inst));
        return m_insts.back().get();
    };

    MipsInst* insertBeforeInst(MipsInst* insertBefore, MipsInst* inst) {
        inst->m_atBlock = this;
        auto finded = std::find_if(m_insts.begin(), m_insts.end(), [insertBefore](std::unique_ptr<MipsInst>& in) {
            return insertBefore == in.get();
        });
        if (finded != m_insts.end()) {
            return m_insts.insert(finded, std::unique_ptr<MipsInst>(inst))->get();
        } else {
            return nullptr;
        }
    }
    void removeInst(MipsInst* inst) {
        m_insts.remove_if([inst](std::unique_ptr<MipsInst>& in) {
            return inst == in.get();
        });
    }
    MipsInst* insertFrontInst(MipsInst* inst) {
        inst->m_atBlock = this;
        m_insts.push_front(std::unique_ptr<MipsInst>(inst));
        return m_insts.front().get();
    }

private:
    BasicBlock* m_irBasicBlock;
    std::list<std::unique_ptr<MipsInst>> m_insts;
    // predecessor and successor
    std::vector<MipsBasicBlock*> m_pred;
    std::array<MipsBasicBlock*, 2> m_succ;
    // branch is translated into multiple instructions
    // points to the first one
    MipsInst* m_controlTransferInst = nullptr;
    // liveness analysis
    // maybe we should use bitset when performance is bad
    std::set<MipsOperand> m_liveuse;
    std::set<MipsOperand> m_def;
    std::set<MipsOperand> m_livein;
    std::set<MipsOperand> m_liveout;
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
    bool isEquiv(const MipsOperand& other) const {
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

class MipsBinary : public MipsInst {
public:
    MipsBinary(MipsCodeType type) :
        MipsInst(type) {}

    bool isIdentity() {
        switch (m_type) {
        case MipsCodeType::Add:
        case MipsCodeType::Sub:
            return m_dst.isEquiv(m_lhs) && m_rhs == MipsOperand::I(0) && m_shift.type == MipsShift::None;
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
public:
    MipsMove(MipsOperand dst, MipsOperand rhs) :
        MipsInst(MipsCodeType::Move), m_dst(dst), m_rhs(rhs) {}

private:
    MipsOperand m_dst;
    MipsOperand m_rhs;
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
    MipsAccess(MipsCodeType type, MipsOperand addr, MipsOperand offset) :
        MipsInst(type), m_addr(addr), m_offset(offset) {}

private:
    MipsOperand m_addr;
    MipsOperand m_offset;
};

class MipsLoad : public MipsAccess {
public:
    MipsLoad(MipsOperand dst, MipsOperand addr, MipsOperand offset) :
        MipsAccess(MipsCodeType::Load, addr, offset), m_dst(dst) {}

private:
    MipsOperand m_dst;
};

class MipsStore : public MipsAccess {
public:
    explicit MipsStore(MipsOperand data, MipsOperand addr, MipsOperand offset) :
        MipsAccess(MipsCodeType::Store, addr, offset), m_data(data) {}

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
    MipsGlobal(SymbolTableItem* sym, MipsOperand dst) :
        MipsInst(MipsCodeType::Global), m_sym(sym), m_dst(dst) {}

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