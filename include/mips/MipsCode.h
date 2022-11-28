#ifndef MIPS_CODE_H
#define MIPS_CODE_H
#pragma once

#include <cassert>
#include <set>
#include <vector>

#include <ir/IR.h>
#include "MipsCodeTypeEnum.h"
#include "MipsRegEnum.h"

class MipsFunc;
class MipsBasicBlock;
class MipsInst;
struct MipsOperand;

enum class MipsCond {
    Any,
    Eq,
    Ne,
    Ge,
    Gt,
    Le,
    Lt
};

inline MipsCond transferCond(IRType type) {
    MipsCond cond;
    if (type == IRType::Gt) {
        cond = MipsCond::Gt;
    } else if (type == IRType::Ge) {
        cond = MipsCond::Ge;
    } else if (type == IRType::Le) {
        cond = MipsCond::Le;
    } else if (type == IRType::Lt) {
        cond = MipsCond::Lt;
    } else if (type == IRType::Eq) {
        cond = MipsCond::Eq;
    } else if (type == IRType::Ne) {
        cond = MipsCond::Ne;
    } else {
        cond = MipsCond::Any;
    }
    return cond;
}

inline MipsCond oppositeCond(MipsCond c) {
    constexpr static MipsCond OPPOSITE[] = {MipsCond::Any, MipsCond::Ne, MipsCond::Eq, MipsCond::Lt,
                                            MipsCond::Le, MipsCond::Gt, MipsCond::Ge};
    return OPPOSITE[static_cast<int>(c)];
}

struct Shift {
    enum class Type {
        Sra, // arithmetic right
        Sll, // logic left
        Srl, // logic right
    } type;
    int shift;

    Shift(Shift::Type t) :
        type(t) {
        shift = 0;
    }

    explicit operator std::string() const {
        std::string name = "";
        switch (type) {
        case Shift::Type::Sra:
            name = "sra";
            break;
        case Shift::Type::Sll:
            name = "sll";
            break;
        case Shift::Type::Srl:
            name = "srl";
            break;
        default: break;
        }
        return name;
    }
};

inline std::ostream& operator<<(std::ostream& os, const Shift& shift) {
    os << std::string(shift);
    return os;
}

inline std::ostream& operator<<(std::ostream& os, const MipsCond& cond) {
    if (cond == MipsCond::Eq) {
        os << "seq";
    } else if (cond == MipsCond::Ne) {
        os << "sne";
    } else if (cond == MipsCond::Any) {
        os << "cmp";
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
    friend void allocateRegister(MipsModule& module);
    friend void computeStackInfo(MipsModule& module);

public:
    MipsFunc* addFunc(MipsFunc* func) {
        m_funcs.push_back(std::unique_ptr<MipsFunc>(func));
        return m_funcs.back().get();
    }
    GlobalVariable* addGlob(GlobalVariable* glob) {
        m_globs.push_back(glob);
        return m_globs.back();
    }
    StringVariable* addStr(StringVariable* str) {
        m_strs.push_back(str);
        return m_strs.back();
    }
    void toCode(std::ostream& os);

private:
    std::vector<std::unique_ptr<MipsFunc>> m_funcs;
    std::vector<GlobalVariable*> m_globs;
    std::vector<StringVariable*> m_strs;
};

class MipsFunc {
    friend void livenessAnalysis(MipsFunc* f);

public:
    explicit MipsFunc(IrFunc* irFunc) :
        m_irFunc(irFunc) {
        m_isMainFunc = irFunc->getFuncItem()->getName() == "main";
    }
    MipsBasicBlock* pushBackBasicBlock(MipsBasicBlock* basicBlock) {
        m_basicBlocks.push_back(std::unique_ptr<MipsBasicBlock>(basicBlock));
        return m_basicBlocks.back().get();
    }
    MipsBasicBlock* getFirstBasicBlock() { return m_basicBlocks.front().get(); }
    IrFunc* getIrFunc() { return m_irFunc; }
    void setVirtualMax(int virtualMax) { m_virtualMax = virtualMax; }
    int getVirtualMax() { return m_virtualMax; }
    int getStackSize() { return m_stackSize; }
    void addStackSize(int addStackSize) { m_stackSize += addStackSize; }
    void toCode(std::ostream& os);
    std::vector<std::unique_ptr<MipsBasicBlock>>& getMipsBasicBlocks() { return m_basicBlocks; }

private:
    std::vector<std::unique_ptr<MipsBasicBlock>> m_basicBlocks;
    IrFunc* m_irFunc;
    // number of virtual registers allocated
    int m_virtualMax = 0;
    // size of stack allocated for local alloca and spilled registers
    int m_stackSize = 0;
    // offset += stack_size + saved_regs * 4;

    bool m_isMainFunc{false};

public:
    // set of callee saved registers used
    std::set<MipsReg> usedCalleeSavedRegs;
    std::vector<MipsInst*> spArgFixup;
};
class MipsInst {
    friend class MipsBasicBlock;

public:
    MipsInst(MipsCodeType type) :
        m_type(type) {}
    MipsBasicBlock* getAtBlock() { return m_atBlock; }
    virtual void toCode(std::ostream& os) = 0;

protected:
    MipsBasicBlock* m_atBlock;
    MipsCodeType m_type;
};

class MipsBasicBlock {
    friend void livenessAnalysis(MipsFunc* f);

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

    MipsInst* insertAfterInst(MipsInst* insertAfter, MipsInst* inst) {
        inst->m_atBlock = this;
        auto finded = std::find_if(m_insts.begin(), m_insts.end(), [insertAfter](std::unique_ptr<MipsInst>& in) {
            return insertAfter == in.get();
        });
        if (finded != m_insts.end()) {
            return m_insts.insert(std::next(finded), std::unique_ptr<MipsInst>(inst))->get();
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
    MipsInst* getFrontInst() { return m_insts.front().get(); }
    void setControlTransferInst(MipsInst* inst) { m_controlTransferInst = inst; }
    MipsInst* getControlTransferInst() { return m_controlTransferInst; }
    void toCode(std::ostream& os);
    std::list<std::unique_ptr<MipsInst>>& getMipsInsts() { return m_insts; }
    BasicBlock* getIrBasicBlock() { return m_irBasicBlock; }

public:
    static IndexMapper<MipsBasicBlock> s_bbMapper;

private:
    BasicBlock* m_irBasicBlock;
    std::list<std::unique_ptr<MipsInst>> m_insts;
    // predecessor and successor
    std::vector<MipsBasicBlock*> m_pred;
    std::array<MipsBasicBlock*, 2> m_succ;
    // branch is translated into multiple instructions
    // points to the first one
    MipsInst* m_controlTransferInst = nullptr;

public:
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
        assert(n >= int(MipsReg::zero) && n <= int(MipsReg::ra));
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
    bool isAllocated() const { return state == State::Allocated; }
    bool isPrecolored() const { return state == State::PreColored; }
    bool isReg() const { return state == State::PreColored || state == State::Allocated || state == State::Virtual; }
    bool needsColor() const { return state == State::Virtual || state == State::PreColored; }

    explicit operator std::string() const {
        std::string prefix = "";
        switch (this->state) {
        case State::PreColored:
        case State::Allocated:
            prefix = "$";
            break;
        case State::Virtual:
            prefix = "v";
            break;
        case State::Immediate:
            break;
        }
        return prefix + std::to_string(this->value);
    }

    friend std::ostream& operator<<(std::ostream& os, const MipsOperand& op) {
        if (op.isAllocated() || op.isPrecolored()) {
            os << s_realRegMap[(MipsReg)op.value];
        } else {
            os << std::string(op);
        }
        return os;
    }
};

namespace std {
template <>
struct hash<MipsOperand> {
    std::size_t operator()(MipsOperand const& m) const noexcept {
        // state (2), value (14)
        return ((((size_t)m.state) << 14u) | (int)m.value) & 65534u;
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
    friend std::pair<std::vector<MipsOperand>, std::vector<MipsOperand>> getDefUse(MipsInst* inst);
    friend std::pair<MipsOperand*, std::vector<MipsOperand*>> getDefUsePtr(MipsInst* inst);

public:
    MipsBinary(MipsCodeType type, MipsOperand dst, MipsOperand lhs, MipsOperand rhs) :
        MipsInst(type), m_dst(dst), m_lhs(lhs), m_rhs(rhs) {}

    bool isIdentity() {
        switch (m_type) {
        case MipsCodeType::Add:
        case MipsCodeType::Sub:
            return m_dst.isEquiv(m_lhs) && m_rhs == MipsOperand::I(0);
        default:
            return false;
        }
    }
    void toCode(std::ostream& os) override;
    std::string instString() {
        switch (m_type) {
        case MipsCodeType::Add: return "add";
        case MipsCodeType::Sub: return "sub";
        case MipsCodeType::Rsb: return "sub";
        case MipsCodeType::Mul: return "mul";
        case MipsCodeType::Div: return "div";
        case MipsCodeType::Mod: return "rem";
        default: return "invalid";
        }
    }

private:
    // Add, Sub, Rsb, Mul, Div, Mod, Lt, Le, Ge, Gt, Eq, Ne, And, Or
    MipsOperand m_dst;
    MipsOperand m_lhs;
    MipsOperand m_rhs;
};

class MipsMove : public MipsInst {
    friend struct MipsMoveCompare;
    friend std::pair<std::vector<MipsOperand>, std::vector<MipsOperand>> getDefUse(MipsInst* inst);
    friend std::pair<MipsOperand*, std::vector<MipsOperand*>> getDefUsePtr(MipsInst* inst);

public:
    MipsMove(MipsOperand dst, MipsOperand rhs) :
        MipsInst(MipsCodeType::Move), m_dst(dst), m_rhs(rhs) {}
    virtual void toCode(std::ostream& os) override;
    MipsOperand getDst() { return m_dst; }
    MipsOperand getRhs() { return m_rhs; }

private:
    MipsOperand m_dst;
    MipsOperand m_rhs;
};

struct MipsMoveCompare {
    bool operator()(MipsMove* const& lhs, const MipsMove* const& rhs) const {
        if (lhs->m_dst != rhs->m_dst) return lhs->m_dst < rhs->m_dst;
        if (lhs->m_rhs != rhs->m_rhs) return lhs->m_rhs < rhs->m_rhs;
        return false;
    }
};

class MipsShift : public MipsInst {
    friend std::pair<std::vector<MipsOperand>, std::vector<MipsOperand>> getDefUse(MipsInst* inst);
    friend std::pair<MipsOperand*, std::vector<MipsOperand*>> getDefUsePtr(MipsInst* inst);

public:
    MipsShift(Shift shiftKind, MipsOperand dst, MipsOperand lhs, int shift) :
        MipsInst(MipsCodeType::Shift), m_shiftKind(shiftKind), m_dst(dst), m_lhs(lhs), m_shift(shift) {}
    virtual void toCode(std::ostream& os) override;

private:
    Shift m_shiftKind;
    MipsOperand m_dst;
    MipsOperand m_lhs;
    int m_shift{0};
};

class MipsBranch : public MipsInst {
    friend std::pair<std::vector<MipsOperand>, std::vector<MipsOperand>> getDefUse(MipsInst* inst);
    friend std::pair<MipsOperand*, std::vector<MipsOperand*>> getDefUsePtr(MipsInst* inst);

public:
    MipsBranch(MipsOperand lhs, MipsOperand rhs, MipsBasicBlock* target) :
        MipsInst(MipsCodeType::Branch), m_lhs(lhs), m_rhs(rhs), m_target(target) {}
    virtual void toCode(std::ostream& os) override;

private:
    MipsOperand m_lhs;
    MipsOperand m_rhs;
    MipsBasicBlock* m_target;
};

class MipsJump : public MipsInst {
    friend std::pair<std::vector<MipsOperand>, std::vector<MipsOperand>> getDefUse(MipsInst* inst);
    friend std::pair<MipsOperand*, std::vector<MipsOperand*>> getDefUsePtr(MipsInst* inst);

public:
    MipsJump(MipsBasicBlock* target) :
        MipsInst(MipsCodeType::Jump), m_target(target) {}
    virtual void toCode(std::ostream& os) override;

private:
    MipsBasicBlock* m_target;
};

class MipsReturn : public MipsInst {
    friend std::pair<std::vector<MipsOperand>, std::vector<MipsOperand>> getDefUse(MipsInst* inst);
    friend std::pair<MipsOperand*, std::vector<MipsOperand*>> getDefUsePtr(MipsInst* inst);

public:
    MipsReturn(MipsFunc* func) :
        MipsInst(MipsCodeType::Return), m_retFunc(func) {}
    virtual void toCode(std::ostream& os) override;

private:
    MipsFunc* m_retFunc;
};

class MipsAccess : public MipsInst {
    friend std::pair<std::vector<MipsOperand>, std::vector<MipsOperand>> getDefUse(MipsInst* inst);
    friend std::pair<MipsOperand*, std::vector<MipsOperand*>> getDefUsePtr(MipsInst* inst);

public:
    MipsAccess(MipsCodeType type, MipsOperand addr, int offset) :
        MipsInst(type), m_addr(addr), m_offset(offset) {}
    virtual void toCode(std::ostream& os) override{};
    void setOffset(int offset) { m_offset = offset; }
    int getOffset() { return m_offset; }

protected:
    MipsOperand m_addr;
    int m_offset;
};

class MipsLoad : public MipsAccess {
    friend std::pair<std::vector<MipsOperand>, std::vector<MipsOperand>> getDefUse(MipsInst* inst);
    friend std::pair<MipsOperand*, std::vector<MipsOperand*>> getDefUsePtr(MipsInst* inst);

public:
    MipsLoad(MipsOperand dst, MipsOperand addr, int offset) :
        MipsAccess(MipsCodeType::Load, addr, offset), m_dst(dst) {}
    virtual void toCode(std::ostream& os) override;

private:
    MipsOperand m_dst;
};

class MipsStore : public MipsAccess {
    friend std::pair<std::vector<MipsOperand>, std::vector<MipsOperand>> getDefUse(MipsInst* inst);
    friend std::pair<MipsOperand*, std::vector<MipsOperand*>> getDefUsePtr(MipsInst* inst);

public:
    explicit MipsStore(MipsOperand data, MipsOperand addr, int offset) :
        MipsAccess(MipsCodeType::Store, addr, offset), m_data(data) {}
    virtual void toCode(std::ostream& os) override;

private:
    MipsOperand m_data;
};

class MipsCompare : public MipsInst {
    friend std::pair<std::vector<MipsOperand>, std::vector<MipsOperand>> getDefUse(MipsInst* inst);
    friend std::pair<MipsOperand*, std::vector<MipsOperand*>> getDefUsePtr(MipsInst* inst);

public:
    explicit MipsCompare(MipsCond cond, MipsOperand dst, MipsOperand lhs, MipsOperand rhs) :
        MipsInst(MipsCodeType::Compare), m_cond(cond), m_dst(dst), m_lhs(lhs), m_rhs(rhs) {}
    virtual void toCode(std::ostream& os) override;

private:
    MipsCond m_cond;
    MipsOperand m_dst;
    MipsOperand m_lhs;
    MipsOperand m_rhs;
};

class MipsCall : public MipsInst {
    friend std::pair<std::vector<MipsOperand>, std::vector<MipsOperand>> getDefUse(MipsInst* inst);
    friend std::pair<MipsOperand*, std::vector<MipsOperand*>> getDefUsePtr(MipsInst* inst);

public:
    explicit MipsCall(FuncItem* func) :
        MipsInst(MipsCodeType::Call), m_func(func) {}
    virtual void toCode(std::ostream& os) override;

private:
    FuncItem* m_func;
};

class MipsSysCall : public MipsInst {
    friend std::pair<std::vector<MipsOperand>, std::vector<MipsOperand>> getDefUse(MipsInst* inst);
    friend std::pair<MipsOperand*, std::vector<MipsOperand*>> getDefUsePtr(MipsInst* inst);

public:
    explicit MipsSysCall() :
        MipsInst(MipsCodeType::Call) {}
    virtual void toCode(std::ostream& os) override;
};

class MipsGlobal : public MipsInst {
    friend std::pair<std::vector<MipsOperand>, std::vector<MipsOperand>> getDefUse(MipsInst* inst);
    friend std::pair<MipsOperand*, std::vector<MipsOperand*>> getDefUsePtr(MipsInst* inst);

public:
    MipsGlobal(SymbolTableItem* sym, MipsOperand dst) :
        MipsInst(MipsCodeType::Global), m_sym(sym), m_dst(dst) {}
    virtual void toCode(std::ostream& os) override;

private:
    MipsOperand m_dst;
    SymbolTableItem* m_sym;
};

class MipsString : public MipsInst {
    friend std::pair<std::vector<MipsOperand>, std::vector<MipsOperand>> getDefUse(MipsInst* inst);
    friend std::pair<MipsOperand*, std::vector<MipsOperand*>> getDefUsePtr(MipsInst* inst);

public:
    MipsString(MipsOperand dst, StringVariable* strVar) :
        MipsInst(MipsCodeType::Global), m_strVar(strVar), m_dst(dst) {}
    virtual void toCode(std::ostream& os) override;

private:
    MipsOperand m_dst;
    StringVariable* m_strVar;
};

std::pair<std::vector<MipsOperand>, std::vector<MipsOperand>> getDefUse(MipsInst* inst);
std::pair<MipsOperand*, std::vector<MipsOperand*>> getDefUsePtr(MipsInst* inst);
#endif