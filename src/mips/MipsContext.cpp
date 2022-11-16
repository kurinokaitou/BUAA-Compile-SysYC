#include <mips/MipsContext.h>
#include <Log.h>
#include <optimize/mips/AllocateRegister.h>

std::vector<std::function<void(MipsModule&)>> MipsContext::s_mipsPasses{allocateRegister};

static inline void insertParallelMv(std::vector<std::pair<MipsOperand, MipsOperand>>& movs, MipsInst* insertBefore) {
    // serialization in any order is okay
    for (auto& pair : movs) {
        auto moveInst = new MipsMove(pair.first, pair.second);
        insertBefore->getAtBlock()->insertBeforeInst(insertBefore, moveInst);
    }
}

void MipsContext::convertMipsCode(IrModule& irModule) {
    for (auto& glob : irModule.m_globalVariables) {
        m_module.addGlob(glob.get());
    }
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
            m_basicBlock = basicBlock.get();
            m_mipsBasicBlock = m_bbMap.at(m_basicBlock);
            for (auto& inst : m_basicBlock->m_insts) {
                convertInst(inst.get());
            }
        }
        for (auto& basicBlock : m_irFunc->m_basicBlocks) {
            m_basicBlock = basicBlock.get();
            m_mipsBasicBlock = m_bbMap.at(m_basicBlock);
            m_lhs.clear();
            m_mv.clear();
            for (auto& inst : m_basicBlock->m_insts) {
                // phi insts must appear at the beginning of bb
                if (auto phiInst = dynamic_cast<PhiInst*>(inst.get())) {
                    convertPhiInst(phiInst);
                } else {
                    break;
                }
            }
            // insert parallel mv at the beginning of current mbb
            insertParallelMv(m_lhs, m_mipsBasicBlock->getFrontInst());
            for (auto& pair : m_mv) {
                auto mbb = m_bbMap[pair.first];
                insertParallelMv(pair.second, mbb->getControlTransferInst());
            }
        }
        m_mipsFunc->setVirtualMax(m_virtualMax);
    }
}

void MipsContext::optimizeMipsCode(int optLevel) {
    for (auto& pass : s_mipsPasses) {
        pass(m_module);
    }
}

MipsOperand MipsContext::genNewVirtualReg() {
    return MipsOperand::V(m_virtualMax++);
}

MipsOperand MipsContext::resolveValue(Value* value) {
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
                        firstBlock->insertFrontInst(new MipsMove(res, MipsOperand::R(MipsReg((int)MipsReg::a0 + i))));
                    } else {
                        // read from sp + (i-4)*4 in entry bb
                        // will be fixed up in later pass
                        auto loadInst = m_mipsBasicBlock->insertFrontInst(new MipsLoad(res, MipsOperand::R(MipsReg::sp), (i - 4) * 4));
                        m_mipsFunc->getSpArgFixup().push_back(loadInst);
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

MipsOperand MipsContext::resolveNoImm(Value* value) {
    if (auto cons = dynamic_cast<ConstValue*>(value)) {
        auto res = genNewVirtualReg();
        auto moveInst = m_mipsBasicBlock->pushBackInst(new MipsMove(res, MipsOperand::I(cons->getImm())));
        return res;
    } else {
        return resolveValue(value);
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

void MipsContext::convertInst(Inst* inst) {
    switch (inst->getIrType()) {
    case IRType::Jump:
        convertJumpInst(dynamic_cast<JumpInst*>(inst));
        break;
    case IRType::Load:
        convertLoadInst(dynamic_cast<LoadInst*>(inst));
        break;
    case IRType::Store:
        convertStoreInst(dynamic_cast<StoreInst*>(inst));
        break;
    case IRType::GetElementPtr:
        convertGetElementPtrInst(dynamic_cast<GetElementPtrInst*>(inst));
        break;
    case IRType::Return:
        convertReturnInst(dynamic_cast<ReturnInst*>(inst));
        break;
    case IRType::Branch:
        convertBranchInst(dynamic_cast<BranchInst*>(inst));
        break;
    case IRType::Call:
        convertCallInst(dynamic_cast<CallInst*>(inst));
        break;
    case IRType::Alloca:
        convertAllocaInst(dynamic_cast<AllocaInst*>(inst));
        break;
    case IRType::Add... IRType::Or:
        convertBinaryInst(dynamic_cast<BinaryInst*>(inst));
        break;
    case IRType::Print:
        // TODO: convert print inst
        break;
    case IRType::Phi:
        break;
    default:
        DBG_ERROR("Can't convert this type of ir instruction!");
        break;
    }
}

void MipsContext::convertJumpInst(JumpInst* inst) {
    auto next = m_bbMap.at(inst->getNextBasicBlock());
    auto jumpinst = m_mipsBasicBlock->pushBackInst(new MipsJump(next));
    m_mipsBasicBlock->setControlTransferInst(jumpinst);
}

void MipsContext::convertLoadInst(LoadInst* inst) {
    auto arr = resolveValue(inst->getArrValue());
    auto index = resolveValue(inst->getIndexValue());
    if (index.isImm()) {
        m_mipsBasicBlock->pushBackInst(new MipsLoad(resolveValue(inst), arr, index.value));
    } else {
        auto vreg = genNewVirtualReg();
        auto addInst = m_mipsBasicBlock->pushBackInst(new MipsBinary(MipsCodeType::Add, vreg, arr, index));
        m_mipsBasicBlock->pushBackInst(new MipsLoad(resolveValue(inst), vreg, 0));
    }
}

void MipsContext::convertStoreInst(StoreInst* inst) {
    auto arr = resolveValue(inst->getArrValue());
    auto data = resolveNoImm(inst->getDataValue());
    auto index = resolveValue(inst->getIndexValue());
    if (index.isImm()) {
        m_mipsBasicBlock->pushBackInst(new MipsStore(data, arr, index.value << 2));
    } else {
        auto vreg = genNewVirtualReg();
        auto addInst = m_mipsBasicBlock->pushBackInst(new MipsBinary(MipsCodeType::Add, vreg, arr, index));
        m_mipsBasicBlock->pushBackInst(new MipsLoad(data, vreg, 0));
    }
}

void MipsContext::convertGetElementPtrInst(GetElementPtrInst* inst) {
    // dst = getelementptr arr, index, multiplier
    auto dst = resolveValue(inst);
    auto arr = resolveValue(inst->getArrValue());
    auto mult = inst->getMultiplier() * 4;
    auto constant = dynamic_cast<ConstValue*>(inst->getIndexValue());

    if (mult == 0 || (constant && constant->getImm() == 0)) {
        // dst <- arr
        m_mipsBasicBlock->pushBackInst(new MipsMove(dst, arr));
    } else if (constant) {
        // dst <- arr + off
        auto off = mult * constant->getImm();
        auto immOperand = MipsOperand::I(off);
        m_mipsBasicBlock->pushBackInst(new MipsBinary(MipsCodeType::Add, dst, arr, immOperand));
    } else {
        // dst <- arr
        auto index = resolveNoImm(inst->getIndexValue());
        auto moveInst = m_mipsBasicBlock->pushBackInst(new MipsMove(dst, arr));
        // dst <- index * mult + dst
        auto vreg = genNewVirtualReg();
        m_mipsBasicBlock->pushBackInst(new MipsBinary(MipsCodeType::Mul, vreg, index, MipsOperand::I(mult)));
        m_mipsBasicBlock->pushBackInst(new MipsBinary(MipsCodeType::Add, dst, vreg, dst));
    }
}

void MipsContext::convertReturnInst(ReturnInst* inst) {
    if (inst->getReturnValue()) {
        auto val = resolveValue(inst->getReturnValue());
        // move val to v0
        auto mvInst = m_mipsBasicBlock->pushBackInst(new MipsMove(MipsOperand::R(MipsReg::v0), val));
        auto returnInst = m_mipsBasicBlock->pushBackInst(new MipsReturn(m_mipsFunc));
        m_mipsBasicBlock->setControlTransferInst(mvInst);
    } else {
        auto returnInst = m_mipsBasicBlock->pushBackInst(new MipsReturn(m_mipsFunc));
        m_mipsBasicBlock->setControlTransferInst(returnInst);
    }
}

void MipsContext::convertBranchInst(BranchInst* inst) {
    MipsOperand compareRes{};
    auto it = m_condMap.find(inst->getCondValue());
    if (it != m_condMap.end()) {
        //m_mipsBasicBlock->setControlTransferInst(it->second.first);
        compareRes = it->second.second;
    } else {
        auto cond = resolveNoImm(inst->getCondValue());
        compareRes = genNewVirtualReg();
        auto cmpInst = m_mipsBasicBlock->pushBackInst(new MipsCompare(MipsCond::Ne, compareRes, cond, MipsOperand::I(0)));
        //m_mipsBasicBlock->setControlTransferInst(cmpInst);
    }
    MipsInst* branch = nullptr;
    if (inst->getTrueBasicBlock() == m_irFunc->nextBasicBlock(m_basicBlock)) {
        branch = m_mipsBasicBlock->pushBackInst(new MipsBranch(compareRes, MipsOperand::I(0), m_bbMap.at(inst->getFalseBasicBlock())));
    } else {
        branch = m_mipsBasicBlock->pushBackInst(new MipsBranch(compareRes, MipsOperand::I(1), m_bbMap.at(inst->getTrueBasicBlock())));
        m_mipsBasicBlock->pushBackInst(new MipsJump(m_bbMap.at(inst->getFalseBasicBlock())));
    }
    m_mipsBasicBlock->setControlTransferInst(branch);
}

void MipsContext::convertCallInst(CallInst* inst) {
    std::vector<MipsOperand> params;
    auto args = inst->getArgsValue();
    int n = args.size();
    for (int i = 0; i < n; i++) {
        if (i < 4) {
            // move args to a0-a3
            auto rhs = resolveValue(args[i]);
            m_mipsBasicBlock->pushBackInst(new MipsMove(MipsOperand::R(MipsReg((int)MipsReg::a0 + i)), rhs));
        } else {
            // store to sp-(n-i)*4
            auto rhs = resolveNoImm(args[i]);
            m_mipsBasicBlock->pushBackInst(new MipsStore(rhs, MipsOperand::R(MipsReg::sp), (-(n - i)) << 2));
        }
    }

    if (n > 4) {
        // sub sp, sp, (n-4)*4
        m_mipsBasicBlock->pushBackInst(new MipsBinary(MipsCodeType::Sub, MipsOperand::R(MipsReg::sp), MipsOperand::R(MipsReg::sp), MipsOperand::I(4 * (n - 4))));
    }
    m_mipsBasicBlock->pushBackInst(new MipsCall(inst->getIrFunc()->m_funcItem));
    if (n > 4) {
        // add sp, sp, (n-4)*4
        m_mipsBasicBlock->pushBackInst(new MipsBinary(MipsCodeType::Add, MipsOperand::R(MipsReg::sp), MipsOperand::R(MipsReg::sp), MipsOperand::I(4 * (n - 4))));
    }

    // return
    if (inst->getIrFunc()->m_funcItem->getReturnValueType() == ValueTypeEnum::INT_TYPE) {
        // has return
        // move v0 to dst
        auto dst = resolveValue(inst);
        m_mipsBasicBlock->pushBackInst(new MipsMove(dst, MipsOperand::R(MipsReg::v0)));
    }
}

void MipsContext::convertAllocaInst(AllocaInst* inst) {
    size_t size = 1;
    auto sym = inst->getSym();
    auto dims = getArrayItemDimensions(sym);
    if (!dims.empty()) {
        size = calArrayDimsSize(dims);
    }
    size *= 4;
    auto dst = resolveValue(inst);
    auto offset = MipsOperand::I(m_mipsFunc->getStackSize());
    m_mipsBasicBlock->pushBackInst(new MipsBinary(MipsCodeType::Add, dst, MipsOperand::R(MipsReg::sp), offset));
    // allocate size on sp
    m_mipsFunc->addStackSize(size);
}

void MipsContext::convertBinaryInst(BinaryInst* inst) {
    MipsOperand rhs{};
    auto rhsIsConst = inst->getRhsValue()->getIrType() == IRType::Const;
    auto lhs = resolveNoImm(inst->getLhsValue());
    // try to use imm
    if (inst->rhsCanBeImm() && rhsIsConst) {
        int imm = dynamic_cast<ConstValue*>(inst->getRhsValue())->getImm();
        rhs = MipsOperand::I(imm); // might be imm or register
    } else {
        rhs = resolveNoImm(inst->getRhsValue());
    }
    if (inst->getIrType() >= IRType::Lt && inst->getIrType() <= IRType::Ne) {
        auto compareRes = resolveValue(inst);
        MipsCond cond = transferCond(inst->getIrType());
        auto uses = inst->getUses();
        auto nextInst = m_basicBlock->nextInst(inst);
        auto cmpInst = m_mipsBasicBlock->pushBackInst(new MipsCompare(cond, compareRes, lhs, rhs));
        if (!uses.empty() && dynamic_cast<BranchInst*>(uses[0]->user) && nextInst == uses[0]->user) { // 存在使用compareRes 的BranchInst
            m_condMap.insert({inst, {cmpInst, compareRes}});
        }
    } else {
        auto mipsCodeType = static_cast<MipsCodeType>(inst->getIrType()); // op.inc部分相同
        m_mipsBasicBlock->pushBackInst(new MipsBinary(mipsCodeType, resolveValue(inst), lhs, rhs));
    }
}

void MipsContext::convertPhiInst(PhiInst* inst) {
    // for each phi:
    // lhs = phi [r1 bb1], [r2 bb2] ...
    // 1. create vreg for each inst
    // 2. add parallel mv (lhs1, ...) = (vreg1, ...)
    // 3. add parallel mv in each bb: (vreg1, ...) = (r1, ...)
    auto vreg = genNewVirtualReg();
    m_lhs.emplace_back(resolveValue(inst), vreg);
    auto incomingValues = inst->getIncomingValues();
    auto predBBs = inst->getAtBlock()->getPreds();
    for (int i = 0; i < incomingValues.size(); i++) {
        auto predBB = predBBs[i];
        auto currBB = m_mipsBasicBlock;
        m_mipsBasicBlock = m_bbMap.at(predBB);
        auto val = resolveValue(incomingValues[i].value);
        m_mipsBasicBlock = currBB;
        m_mv[predBB].emplace_back(vreg, val);
    }
}