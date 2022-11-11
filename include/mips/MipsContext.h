#ifndef MIPS_CONTEXT_H
#define MIPS_CONTEXT_H
#include <map>

#include "MipsCode.h"
class MipsContext {
    friend class CodeGenerator;

public:
    void convertMipsCode(IrModule& irModule);

private:
    void mapBasicBlocks();
    MipsOperand genNewVirtualReg();
    MipsOperand resolveValue(Value* value, MipsBasicBlock* mbb);
    MipsOperand resolveNoImm(Value* value, MipsBasicBlock* mbb);
    void convertInstruction(Inst* inst);
    void convertPhiInstruction(PhiInst* inst);

private:
    MipsModule m_module;
    IrFunc* m_irFunc;
    MipsFunc* m_mipsFunc;
    //  machine bb 1-to-1
    std::map<BasicBlock*, MipsBasicBlock*> m_bbMap;
    // map value to MipsOperand
    std::map<Value*, MipsOperand> m_valMap;
    // map globalVariable to MipsOperand
    std::map<GlobalVariable*, MipsOperand> m_globMap;
    // map paramVariable to MipsOperand
    std::map<ParamVariable*, MipsOperand> m_paramMap;
    // map use to simplify branch cond
    std::map<Value*, std::pair<MipsInst*, MipsCond>> m_condMap;
    // virtual registers
    int m_virtualMax = 0;
};

#endif