#ifndef MIPS_CONTEXT_H
#define MIPS_CONTEXT_H
#include <map>
#include <functional>

#include "MipsCode.h"
class MipsContext {
    friend class CodeGenerator;

public:
    void convertMipsCode(IrModule& irModule);
    void optimizeMipsCode(int optLevel);

private:
    void mapBasicBlocks();
    MipsOperand genNewVirtualReg();
    MipsOperand resolveValue(Value* value);
    MipsOperand resolveNoImm(Value* value);
    void convertInst(Inst* inst);
    void convertJumpInst(JumpInst* inst);
    void convertLoadInst(LoadInst* inst);
    void convertStoreInst(StoreInst* inst);
    void convertGetElementPtrInst(GetElementPtrInst* inst);
    void convertReturnInst(ReturnInst* inst);
    void convertBinaryInst(BinaryInst* inst);
    void convertBranchInst(BranchInst* inst);
    void convertCallInst(CallInst* inst);
    void convertAllocaInst(AllocaInst* inst);
    void convertPhiInst(PhiInst* inst);
    void convertPrintInst(PrintInst* inst);

private:
    MipsModule m_module;
    IrFunc* m_irFunc;
    MipsFunc* m_mipsFunc;
    MipsBasicBlock* m_mipsBasicBlock;
    BasicBlock* m_basicBlock;
    //  machine bb 1-to-1
    std::map<BasicBlock*, MipsBasicBlock*> m_bbMap;
    // map value to MipsOperand
    std::map<Value*, MipsOperand> m_valMap;
    // map globalVariable to MipsOperand
    std::map<GlobalVariable*, MipsOperand> m_globMap;
    // map paramVariable to MipsOperand
    std::map<ParamVariable*, MipsOperand> m_paramMap;
    // map use to simplify branch cond
    std::map<Value*, std::pair<MipsInst*, MipsOperand>> m_condMap;
    // (lhs, vreg) assignments
    std::vector<std::pair<MipsOperand, MipsOperand>> m_lhs;
    // each bb has a list of (vreg, rhs) parallel moves
    std::map<BasicBlock*, std::vector<std::pair<MipsOperand, MipsOperand>>> m_mv;
    // virtual registers
    int m_virtualMax = 0;
    // optimize passes
public:
    static std::vector<std::function<void(MipsModule&)>> s_mipsPasses;
};

#endif