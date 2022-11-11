#include <mips/MipsContext.h>

void MipsContext::convertMipsCode(IrModule& irModule) {
    for (auto& irFunc : irModule.m_funcs) {
        m_bbMap.clear();
        m_valMap.clear();
        m_globMap.clear();
        m_paramMap.clear();
        m_condMap.clear();
        m_virtualMax = 0;
        m_irFunc = irFunc.get();
        m_mipsFunc = m_module.addFunc(new MipsFunc(irFunc.get()));
        mapBasicBlocks();
        for (auto& basicBlock : m_irFunc->m_basicBlocks) {
            auto bb = basicBlock.get();
            auto mbb = m_bbMap.at(bb);
            for (auto& inst : bb->m_insts) {
                convertInstruction(inst.get());
            }
        }
        for (auto& basicBlock : m_irFunc->m_basicBlocks) {
            auto bb = basicBlock.get();
            auto mbb = m_bbMap.at(bb);
            for (auto& inst : bb->m_insts) {
                // phi insts must appear at the beginning of bb
                if (auto phiInst = dynamic_cast<PhiInst*>(inst.get())) {
                    convertPhiInstruction(phiInst);
                } else {
                    break;
                }
            }
        }
        m_mipsFunc->setVirtualMax(m_virtualMax);
    }
}

MipsOperand MipsContext::genNewVirtualReg() {
    return MipsOperand::V(m_virtualMax++);
}

MipsOperand MipsContext::resolveValue(Value* value, MipsBasicBlock* mbb) {
    auto type = value->getIrType();
    if (type == IRType::Param) {
        auto param = dynamic_cast<ParamVariable*>(value);
        auto paramItem = param->getParamItem();
        auto funcItem = m_irFunc->getFuncItem();
        auto it = m_paramMap.find(param);
        if (it == m_paramMap.end()) {
            // allocate virtual reg
            auto res = genNewVirtualReg();
            m_valMap.insert({value, res});
            m_paramMap.insert({param, res});
            auto& params = funcItem->getParams();
            for (int i = 0; i < params.size(); i++) {
                if (params[i] == paramItem) {
                    if (i < 4) {
                        // a0-a3
                        // copy param to vreg in entry bb
                        auto firstBlock = m_mipsFunc->getFirstBasicBlock();
                        firstBlock->insertFrontInst(new MipsMove(MipsOperand::R((MipsReg)i), res));
                    } else {
                        // read from sp + (i-4)*4 in entry bb
                        // will be fixed up in later pass
                        auto vreg = genNewVirtualReg();
                        mbb->insertFrontInst(new MipsLoad(res, MipsOperand::R(MipsReg::sp), vreg));
                        auto mvInst = mbb->insertFrontInst(new MipsMove(vreg, MipsOperand::I((i - 4) * 4)));
                        m_mipsFunc->getSpArgFixup().push_back(mvInst);
                    }
                    break;
                }
            }
            return res;
        } else {
            return it->second;
        }
    } else if (type == IRType::Global) {
        auto global = dynamic_cast<GlobalVariable*>(value);
        auto it = m_globMap.find(global);
        if (it == m_globMap.end()) {
            // load global addr in entry bb
            //          if (glo)
            auto res = genNewVirtualReg();
            auto firstBlock = m_mipsFunc->getFirstBasicBlock();
            auto globInst = firstBlock->insertFrontInst(new MipsGlobal(global->getGlobalItem(), res));
            m_valMap.insert({value, res});
            m_globMap.insert({global, res});
            return res;
        } else {
            return it->second;
        }
    } else if (type == IRType::Const) {
        auto cons = dynamic_cast<ConstValue*>(value);
        return MipsOperand::I(cons->getImm());
    } else {
        auto it = m_valMap.find(value);
        if (it == m_valMap.end()) {
            // allocate virtual reg
            auto res = genNewVirtualReg();
            m_valMap.insert({value, res});
            return res;
        } else {
            return it->second;
        }
    }
}

MipsOperand MipsContext::resolveNoImm(Value* value, MipsBasicBlock* mbb) {
    if (auto cons = dynamic_cast<ConstValue*>(value)) {
        auto res = genNewVirtualReg();
        auto moveInst = mbb->pushBackInst(new MipsMove(res, MipsOperand::I(cons->getImm())));
        return res;
    } else {
        return resolveValue(value, mbb);
    }
}

void MipsContext::mapBasicBlocks() {
    m_bbMap.clear();
    for (auto& bb : m_irFunc->m_basicBlocks) {
        auto mbb = m_mipsFunc->pushBackBasicBlock(new MipsBasicBlock(bb.get()));
        m_bbMap.insert({bb.get(), mbb});
    }
    // maintain pred and succ
    for (auto& bb : m_irFunc->m_basicBlocks) {
        auto mbb = m_bbMap.at(bb.get());
        mbb->getPreds().reserve(bb->getPreds().size());
        // at most two successor
        auto succ = bb->getSuccs();
        for (int i = 0; i < 2; i++) {
            if (succ[i]) {
                mbb->getSuccs()[i] = m_bbMap[succ[i]];
            } else {
                mbb->getSuccs()[i] = nullptr;
            }
        }
        for (auto& pred : bb->getPreds()) {
            mbb->getPreds().push_back(m_bbMap[pred]);
        }
    }
}

void MipsContext::convertInstruction(Inst* inst) {
}

void MipsContext::convertPhiInstruction(PhiInst* inst) {
}