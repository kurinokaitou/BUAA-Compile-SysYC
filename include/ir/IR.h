#ifndef IR_H
#define IR_H
#include <vector>
#include <list>
#include <cassert>
#include <memory>
#include <set>
#include <unordered_set>
#include <map>

#include "IRTypeEnum.h"
#include <symbol/SymbolTableItem.h>
#include <symbol/ValueType.h>
#include <Utils.h>

static void printDimensions(std::ostream& os, std::vector<size_t>& dims);

struct Use;
/*
declare i32 @getint()          ; 读取一个整数
declare void @putint(i32)      ; 输出一个整数
declare void @putch(i32)       ; 输出一个字符
declare void @putstr(i8*)      ; 输出字符串
*/
constexpr static const int FUNC_NUM = 4;
static const std::string BUILTIN_FUNCS[FUNC_NUM][2] = {
    {"getint", ""},
    {"putint", "i32"},
    {"putch", "i32"},
    {"putstr", "i8*"},
};
static const ValueTypeEnum BUILTIN_FUNCS_RETURN_TYPE[FUNC_NUM] = {ValueTypeEnum::INT_TYPE, ValueTypeEnum::VOID_TYPE, ValueTypeEnum::VOID_TYPE, ValueTypeEnum::VOID_TYPE};

constexpr static const char* LLVM_OPS[14] = {
    /* Add = */ "add",
    /* Sub = */ "sub",
    /* Rsb = */ nullptr,
    /* Mul = */ "mul",
    /* Div = */ "sdiv",
    /* Mod = */ "srem",
    /* Lt = */ "icmp slt",
    /* Le = */ "icmp sle",
    /* Ge = */ "icmp sge",
    /* Gt = */ "icmp sgt",
    /* Eq = */ "icmp eq",
    /* Ne = */ "icmp ne",
    /* And = */ "and",
    /* Or = */ "or",
};

constexpr static std::pair<IRType, IRType>
    swapableOperators[11] = {
        {IRType::Add, IRType::Add},
        {IRType::Sub, IRType::Rsb},
        {IRType::Mul, IRType::Mul},
        {IRType::Lt, IRType::Gt},
        {IRType::Le, IRType::Ge},
        {IRType::Gt, IRType::Lt},
        {IRType::Ge, IRType::Le},
        {IRType::Eq, IRType::Eq},
        {IRType::Ne, IRType::Ne},
        {IRType::And, IRType::And},
        {IRType::Or, IRType::Or},
};

template <class T>
class IndexMapper {
public:
    int alloc() { return m_index++; }
    int get(T* t) {
        auto res = m_mapper.insert({t, m_index});
        auto it = res.first;
        auto inserted = res.second;
        m_index += inserted;
        return it->second;
    }
    void reset() {
        m_index = 0;
        m_mapper.clear();
    }

private:
    std::map<T*, int> m_mapper;
    int m_index{0};
};

class Value {
public:
    explicit Value(IRType type) :
        m_type(type){};
    virtual ~Value() {}
    void addUse(Use* use) { m_uses.push_back(use); };
    void removeUse(Use* use) {
        for (auto it = m_uses.begin(); it != m_uses.end(); it++) {
            if (*it == use) {
                it = m_uses.erase(it);
                break;
            }
        }
    };
    std::vector<Use*>& getUses() { return m_uses; }
    IRType getIrType() const { return m_type; }
    virtual void printValue(std::ostream& os) {
        os << "%_x" << s_valueMapper.get(this);
    };
    virtual bool isGlob() { return false; }
    static IndexMapper<Value> s_valueMapper;

protected:
    IRType m_type;
    std::vector<Use*> m_uses;
};
class BasicBlock;
class Inst : public Value {
    friend class BasicBlock;

public:
    Inst(IRType type) :
        Value(type){};
    virtual ~Inst() {}
    virtual std::vector<Use*> getOperands() { return {}; };
    virtual void toCode(std::ostream& os) { os << "vacantInst"; };
    void printValue(std::ostream& os) override {
        Value::printValue(os);
    }
    BasicBlock* getAtBlock() { return m_atBlock; }

protected:
    BasicBlock* m_atBlock;
};

class BasicBlock {
    friend class IrFunc;
    friend class MipsContext;

public:
    BasicBlock() = default;
    std::vector<BasicBlock*>& getPreds() { return m_pred; }
    std::array<BasicBlock*, 2> getSuccs();
    Inst* pushBackInst(Inst* inst) {
        inst->m_atBlock = this;
        m_insts.push_back(std::unique_ptr<Inst>(inst));
        return m_insts.back().get();
    };
    void removeInst(Inst* inst) {
        m_insts.remove_if([inst](std::unique_ptr<Inst>& in) {
            return inst == in.get();
        });
    }
    Inst* insertFrontInst(Inst* inst) {
        inst->m_atBlock = this;
        m_insts.push_front(std::unique_ptr<Inst>(inst));
        return m_insts.front().get();
    }
    Inst* nextInst(Inst* inst) {
        auto finded = std::find_if(m_insts.begin(), m_insts.end(), [inst](std::unique_ptr<Inst>& in) {
            return in.get() == inst;
        });
        if (finded != m_insts.end() && finded != std::prev(m_insts.end())) {
            return std::next(finded)->get();
        } else {
            DBG_ERROR("Can not find next inst of current inst!");
            return inst;
        }
    }
    bool valid();

private:
    std::vector<BasicBlock*> m_pred;
    bool m_vis{false};
    std::list<std::unique_ptr<Inst>> m_insts;

public:
    BasicBlock* idom;
    std::unordered_set<BasicBlock*> domBy; // 支配它的节点集
    std::vector<BasicBlock*> doms;         // 它支配的节点集
    int domLevel;                          // dom树中的深度，根深度为0
    bool vis;
    static IndexMapper<BasicBlock> s_bbMapper;
};

class IrModule;
class IrFunc {
    friend class IrModule;
    friend class CallInst;
    friend class MipsContext;

public:
    explicit IrFunc(FuncItem* funcItem) :
        m_funcItem(funcItem) {}
    explicit IrFunc(const std::string& funcName, const std::string& builtinArgType, ValueTypeEnum retType) :
        m_isBuiltin(true), m_builtinArgType(builtinArgType) {
        m_funcItem = new FuncItem(funcName, retType);
    }
    BasicBlock* firstBasicBlock() {
        return m_basicBlocks.front().get();
    }
    BasicBlock* pushBackBasicBlock(BasicBlock* basicBlock) {
        m_basicBlocks.push_back(std::unique_ptr<BasicBlock>(basicBlock));
        return m_basicBlocks.back().get();
    }
    BasicBlock* nextBasicBlock(BasicBlock* block) {
        auto finded = std::find_if(m_basicBlocks.begin(), m_basicBlocks.end(), [block](std::unique_ptr<BasicBlock>& b) {
            return b.get() == block;
        });
        if (finded != m_basicBlocks.end() && finded != std::prev(m_basicBlocks.end())) {
            return std::next(finded)->get();
        } else {
            DBG_ERROR("Can not find next block of current block!");
            return block;
        }
    }
    std::vector<std::unique_ptr<BasicBlock>>& getBasicBlocks() { return m_basicBlocks; }
    FuncItem* getFuncItem() { return m_funcItem; }
    bool hasReturn() { return m_funcItem->getReturnValueType() != ValueTypeEnum::VOID_TYPE; }
    void toCode(std::ostream& os);
    IrModule* getFromModule() { return m_fromModule; }
    void clearAllVisitFlag() {
        for (auto& bb : m_basicBlocks) {
            bb->vis = false;
        }
    }

private:
    FuncItem* m_funcItem{nullptr};
    IrModule* m_fromModule{nullptr};
    std::vector<std::unique_ptr<BasicBlock>> m_basicBlocks;
    bool m_isBuiltin{false};
    std::string m_builtinArgType;

public:
    std::set<IrFunc*> callee;
    std::set<IrFunc*> caller;
};

class GlobalVariable : public Value {
    friend class IrModule;

public:
    explicit GlobalVariable(SymbolTableItem* globalItem) :
        Value(IRType::Global), m_globalItem(globalItem) {}
    virtual ~GlobalVariable() {}
    virtual bool isGlob() override { return true; }
    virtual void printValue(std::ostream& os) override {
        os << "%g_" << m_globalItem->getName();
    }
    SymbolTableItem* getGlobalItem() { return m_globalItem; }

private:
    SymbolTableItem* m_globalItem;
};

class StringVariable : public Value {
    friend class IrModule;

public:
    explicit StringVariable(std::string name, std::string str) :
        Value(IRType::Global), m_name(name), m_str(str) {
        replaceAll(m_str, "\"", "");
        int num = replaceAll(m_str, "\\n", "\\0a");
        m_len = m_str.size() - num * 2 + 1;
        m_str += "\\00";
    }
    virtual ~StringVariable() {}
    virtual void printValue(std::ostream& os) override {
        os << "@" << m_name;
    }
    void printStrType(std::ostream& os);
    void printString(std::ostream& os);

private:
    std::string m_name;
    std::string m_str;
    int m_len;
};

class ParamVariable : public Value {
public:
    explicit ParamVariable(SymbolTableItem* paramItem) :
        Value(IRType::Param), m_paramItem(paramItem) {}
    virtual ~ParamVariable() {}
    void printValue(std::ostream& os) override {
        os << "%" << m_paramItem->getName();
    }
    SymbolTableItem* getParamItem() { return m_paramItem; }

private:
    SymbolTableItem* m_paramItem;
};

class IrModule {
    friend class MipsModule;
    friend class MipsContext;

public:
    IrModule() {
        for (int i = 0; i < FUNC_NUM; i++) {
            s_builtinFuncs.insert({BUILTIN_FUNCS[i][0], new IrFunc(BUILTIN_FUNCS[i][0], BUILTIN_FUNCS[i][1], BUILTIN_FUNCS_RETURN_TYPE[i])});
        }
    }
    IrFunc* addFunc(IrFunc* func) {
        m_funcs.push_back(std::unique_ptr<IrFunc>(func));
        func->m_fromModule = this;
        return m_funcs.back().get();
    }

    GlobalVariable* addGlobalVar(SymbolTableItem* item) {
        m_globalVariables.push_back(std::unique_ptr<GlobalVariable>(new GlobalVariable(item)));
        return m_globalVariables.back().get();
    }

    StringVariable* addStrVar(const std::string& str) {
        std::string name = ".str" + std::to_string(m_strVariables.size() + 1);
        m_strVariables.push_back(std::unique_ptr<StringVariable>(new StringVariable(name, str)));
        return m_strVariables.back().get();
    }

    void calPredSucc();

    IrFunc* getFunc(FuncItem* funcItem);
    static IrFunc* getBuiltinFunc(const std::string& funcName);
    std::vector<std::unique_ptr<GlobalVariable>>& getGlobalVariables() { return m_globalVariables; }
    void toCode(std::ostream& os, bool isTest);

public:
    static std::map<std::string, IrFunc*> s_builtinFuncs;
    static std::set<IrFunc*> s_usedBuiltinFuncs;

private:
    std::vector<std::unique_ptr<IrFunc>> m_funcs;
    std::vector<std::unique_ptr<GlobalVariable>> m_globalVariables;
    std::vector<std::unique_ptr<StringVariable>> m_strVariables;
};

struct Use {
    Value* value{nullptr};
    Inst* user{nullptr};
    Use() = default;
    Use(Value* v, Inst* u) :
        value(v), user(u) {
        if (v) v->addUse(this);
    }
    Use(const Use& rhs) :
        value(rhs.value), user(rhs.user) {
        if (value) value->addUse(this);
    }
    Use& operator=(const Use& rhs) {
        if (this != &rhs) {
            assert(user == rhs.user);
            set(rhs.value);
        }
        return *this;
    }
    void set(Value* v) {
        if (value) value->removeUse(this);
        value = v;
        if (v) v->addUse(this);
    }
    ~Use() {
        if (value) value->removeUse(this);
    }
};

class BinaryInst : public Inst {
public:
    BinaryInst(IRType type, Value* lhs, Value* rhs) :
        Inst(type), m_lhs(lhs, this), m_rhs(rhs, this) {}
    virtual ~BinaryInst() {}
    bool rhsCanBeImm() {
        // Add, Sub, Rsb, Mul, Div, Mod, Lt, Le, Ge, Gt, Eq, Ne, And, Or
        return (m_type >= IRType::Add && m_type <= IRType::Rsb) || (m_type >= IRType::Lt && m_type <= IRType::Or);
    }

    bool swapOperand() {
        for (auto pair : swapableOperators) {
            auto& before = pair.first;
            auto& after = pair.second;
            if (m_type == before) {
                m_type = after;
                Value *l = m_lhs.value, *r = m_rhs.value;
                l->removeUse(&m_lhs);
                r->removeUse(&m_rhs);
                l->addUse(&m_rhs);
                r->addUse(&m_lhs);
                std::swap(m_lhs.value, m_rhs.value);
                return true;
            }
        }
        return false;
    }

    virtual std::vector<Use*> getOperands() override { return {&m_lhs, &m_rhs}; };
    virtual void toCode(std::ostream& os) override;
    Value* getLhsValue() { return m_lhs.value; }
    Value* getRhsValue() { return m_rhs.value; }

private:
    Use m_lhs;
    Use m_rhs;
};

class ConstValue : public Value {
public:
    static ConstValue* get(int imm) {
        auto pair = POOL.insert({imm, nullptr});
        auto& inserted = pair.second;
        auto& it = pair.first;
        if (inserted) it->second = new ConstValue(imm);
        return it->second;
    }
    virtual ~ConstValue() {}
    const int getImm() const { return m_imm; }
    void printValue(std::ostream& os) override {
        os << m_imm;
    }

private:
    const int m_imm;
    static std::map<int, ConstValue*> POOL;

    // use ConstValue::get instead
    explicit ConstValue(int imm) :
        Value(IRType::Const), m_imm(imm) {}
};

struct BranchInst : public Inst {
    friend class BasicBlock;

public:
    explicit BranchInst(Value* cond, BasicBlock* left, BasicBlock* right) :
        Inst(IRType::Branch), m_cond(cond, this), m_left(left), m_right(right) {}
    virtual ~BranchInst() {}
    virtual std::vector<Use*> getOperands() override { return {&m_cond}; };
    virtual void toCode(std::ostream& os) override;
    Value* getCondValue() { return m_cond.value; };
    BasicBlock* getTrueBasicBlock() { return m_left; }
    BasicBlock* getFalseBasicBlock() { return m_right; }

private:
    Use m_cond;
    // true
    BasicBlock* m_left;
    // false
    BasicBlock* m_right;
};

class JumpInst : public Inst {
    friend class BasicBlock;

public:
    explicit JumpInst(BasicBlock* next) :
        Inst(IRType::Jump), m_next(next) {}
    virtual ~JumpInst() {}
    virtual std::vector<Use*> getOperands() override { return {}; };
    virtual void toCode(std::ostream& os) override;
    BasicBlock* getNextBasicBlock() { return m_next; }

private:
    BasicBlock* m_next;
};

class ReturnInst : public Inst {
    friend class BasicBlock;

public:
    explicit ReturnInst(Value* ret) :
        Inst(IRType::Return), m_ret(ret, this) {}
    virtual ~ReturnInst() {}
    virtual std::vector<Use*> getOperands() override { return {&m_ret}; };
    virtual void toCode(std::ostream& os) override;
    Value* getReturnValue() { return m_ret.value; }

private:
    Use m_ret;
};

class AccessInst : public Inst {
    friend class BasicBlock;

public:
    explicit AccessInst(IRType type, SymbolTableItem* lhs_sym, Value* arr, Value* index) :
        Inst(type), m_lhsSym(lhs_sym), m_arr(arr, this), m_index(index, this) {}
    virtual ~AccessInst(){};
    virtual std::vector<Use*> getOperands() override { return {&m_arr, &m_index}; };
    virtual void toCode(std::ostream& os) override { Inst::toCode(os); }
    virtual void printValue(std::ostream& os) override { Inst::printValue(os); };
    SymbolTableItem* getLhsSym() { return m_lhsSym; }
    Value* getArrValue() { return m_arr.value; }
    Value* getIndexValue() { return m_index.value; }

protected:
    SymbolTableItem* m_lhsSym;
    Use m_arr;
    Use m_index;
};

class GetElementPtrInst : public AccessInst {
    friend class BasicBlock;

public:
    explicit GetElementPtrInst(SymbolTableItem* lhsSym, Value* arr, Value* index, int multiplier) :
        AccessInst(IRType::GetElementPtr, lhsSym, arr, index), m_multiplier(multiplier) {}
    virtual ~GetElementPtrInst() {}
    virtual std::vector<Use*> getOperands() override { return AccessInst::getOperands(); };
    virtual void toCode(std::ostream& os) override;
    int getMultiplier() { return m_multiplier; }

private:
    int m_multiplier;
};

class LoadInst : public AccessInst {
    friend class BasicBlock;

public:
    explicit LoadInst(SymbolTableItem* lhsSym, Value* arr, Value* index) :
        AccessInst(IRType::Load, lhsSym, arr, index), m_memToken(nullptr, this) {}
    virtual ~LoadInst() {}
    virtual std::vector<Use*> getOperands() override { return {&m_arr, &m_memToken}; };
    virtual void toCode(std::ostream& os) override;

private:
    Use m_memToken; // 由memdep pass计算
};

class StoreInst : public AccessInst {
    friend class BasicBlock;

public:
    explicit StoreInst(SymbolTableItem* lhsSym, Value* arr, Value* data, Value* index) :
        AccessInst(IRType::Store, lhsSym, arr, index), m_data(data, this) {}
    virtual ~StoreInst() {}
    virtual std::vector<Use*> getOperands() override { return {&m_arr, &m_data}; };
    virtual void toCode(std::ostream& os) override;
    virtual void printValue(std::ostream& os) override {
        os << "store" << s_valueMapper.get(this);
    }
    Value* getDataValue() { return m_data.value; }

private:
    Use m_data;
};

class CallInst : public Inst {
public:
    explicit CallInst(IrFunc* func, std::vector<Value*> args) :
        Inst(IRType::Call), m_func(func) {
        m_args.reserve(args.size());
        for (auto& arg : args) {
            m_args.emplace_back(arg, this);
        }
    }
    virtual ~CallInst() {}
    virtual std::vector<Use*> getOperands() override {
        std::vector<Use*> usePtrs;
        usePtrs.reserve(m_args.size());
        for (auto& use : m_args) {
            usePtrs.push_back(&use);
        }
        return usePtrs;
    };
    virtual void toCode(std::ostream& os) override;

    IrFunc* getIrFunc() { return m_func; }
    std::vector<Value*> getArgsValue() {
        std::vector<Value*> values;
        values.reserve(m_args.size());
        for (auto& arg : m_args) {
            values.push_back(arg.value);
        }
        return values;
    }

private:
    IrFunc* m_func;
    std::vector<Use> m_args;
};

class AllocaInst : public Inst {
public:
    AllocaInst(SymbolTableItem* sym) :
        Inst(IRType::Alloca), m_sym(sym) {}
    virtual ~AllocaInst() {}
    virtual std::vector<Use*> getOperands() override { return {}; };
    virtual void toCode(std::ostream& os) override;
    SymbolTableItem* getSym() { return m_sym; }

private:
    SymbolTableItem* m_sym;
};

class PhiInst : public Inst {
public:
    explicit PhiInst(BasicBlock* atBlock) :
        Inst(IRType::Phi) {
        m_atBlock = atBlock;
        int predsNum = m_atBlock->getPreds().size();
        m_incomingValues.reserve(predsNum);
        for (int i = 0; i < predsNum; i++) {
            // 在new PhiInst的时候还不知道它用到的value是什么，先填nullptr，后面再用Use::set填上
            m_incomingValues.emplace_back(nullptr, this);
        }
    }
    virtual ~PhiInst() {}
    std::vector<Use>& getIncomingValues() { return m_incomingValues; }
    virtual std::vector<Use*> getOperands() override {
        std::vector<Use*> usePtrs;
        usePtrs.reserve(m_incomingValues.size());
        for (auto& use : m_incomingValues) {
            usePtrs.push_back(&use);
        }
        return usePtrs;
    };
    virtual void toCode(std::ostream& os) override;

private:
    std::vector<Use> m_incomingValues;
};

class PrintInst : public Inst {
public:
    explicit PrintInst(const std::vector<StringVariable*>& strParts, std::vector<Value*> args) :
        Inst(IRType::Print) {
        m_args.reserve(args.size());
        for (auto& arg : args) {
            m_args.emplace_back(arg, this);
        }
        m_strParts.assign(strParts.begin(), strParts.end());
    }
    virtual ~PrintInst() {}
    virtual std::vector<Use*> getOperands() override {
        std::vector<Use*> usePtrs;
        usePtrs.reserve(m_args.size());
        for (auto& use : m_args) {
            usePtrs.push_back(&use);
        }
        return usePtrs;
    };
    virtual void toCode(std::ostream& os) override;

private:
    void printPutInt(const Use& arg, std::ostream& os);
    void printPutStr(StringVariable* strPart, std::ostream& os);

private:
    std::vector<Use> m_args;
    std::vector<StringVariable*> m_strParts;
};

struct IrContext {
    IrModule module;
    IrFunc* function;
    BasicBlock* basicBlock;
    std::vector<std::pair<BasicBlock*, BasicBlock*>> loopStk; // <continue, break>
};

#endif