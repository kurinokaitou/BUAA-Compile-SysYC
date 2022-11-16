#include <mips/MipsCode.h>
#include <ir/Printf.h>
IndexMapper<MipsBasicBlock> MipsBasicBlock::s_bbMapper;

static void moveStack(bool enter, int offset, std::ostream& os, bool hasTab = false) {
    os << (hasTab ? "\t" : "") << (enter ? "subu " : "addu ") << "$sp, $sp, " << offset << std::endl;
}

void MipsModule::toCode(std::ostream& os) {
    int size = 0;
    os << ".data" << std::endl;
    for (auto& glob : m_globs) {
        auto globItem = glob->getGlobalItem();
        auto values = getItemValues(globItem);
        auto dims = getArrayItemDimensions(globItem);
        auto dimsSize = calArrayDimsSize(dims);
        os << globItem->getName() << ":" << std::endl;
        bool hasInit = globItem->hasInit();
        int i = 0;
        for (auto& value : values) {
            os << "\t.word " << (hasInit ? value : 0) << "\n";
            i++;
        }
        for (; i < dimsSize; i++) {
            os << "\t.word " << 0 << "\n";
        }
    }
    for (auto& str : m_strs) {
        os << str->getName() << ": .ascii \"" << str->getRawStr() << "\"\n";
    }
    os << std::endl;
    os << ".text" << std::endl;
    MipsFunc* mainFunc = nullptr;
    for (auto& func : m_funcs) {
        if (func->getIrFunc()->getFuncItem()->getName() == "main") {
            mainFunc = func.get();
            break;
        }
    }
    mainFunc->toCode(os);
    for (auto& func : m_funcs) {
        if (func.get() != mainFunc) {
            func->toCode(os);
        }
    }
    os << s_rawMipsPrint << std::endl;
}

void MipsFunc::toCode(std::ostream& os) {
    os << m_irFunc->getFuncItem()->getName() << ":" << std::endl;
    if (!m_isMainFunc) {
        auto savedRegSize = usedCalleeSavedRegs.size();
        if (savedRegSize) {
            os << "\tsubu $sp, $sp, " << savedRegSize * 4 << "\n";
            int lastStack = savedRegSize * 4;
            for (auto& used : usedCalleeSavedRegs) {
                lastStack -= 4;
                os << "\tsw " << MipsOperand::R(used) << ", " << lastStack << "($sp)\n";
            }
        }
        if (m_stackSize != 0) {
            moveStack(true, m_stackSize, os, true);
        }
    }
    for (auto& mbb : m_basicBlocks) {
        MipsBasicBlock::s_bbMapper.get(mbb.get());
    }
    for (auto& mbb : m_basicBlocks) {
        mbb->toCode(os);
    }
    os << std::endl;
}

void MipsBasicBlock::toCode(std::ostream& os) {
    os << "_b" << MipsBasicBlock::s_bbMapper.get(this) << ":" << std::endl;
    for (auto& inst : m_insts) {
        os << "\t";
        inst->toCode(os);
    }
    os << std::endl;
}

void MipsGlobal::toCode(std::ostream& os) {
    os << "la " << m_dst << ", " << m_sym->getName() << std::endl;
}

void MipsString::toCode(std::ostream& os) {
    os << "la " << m_dst << ", " << m_strVar->getName() << std::endl;
}

void MipsMove::toCode(std::ostream& os) {
    if (m_rhs.isImm()) {
        os << "li " << m_dst << ", " << m_rhs << std::endl;
    } else {
        os << "move " << m_dst << ", " << m_rhs << std::endl;
    }
}
void MipsBranch::toCode(std::ostream& os) {
    os << "beq " << m_lhs << ", " << m_rhs << ", "
       << "_b" << MipsBasicBlock::s_bbMapper.get(m_target) << std::endl;
}
void MipsJump::toCode(std::ostream& os) {
    os << "j "
       << "_b" << MipsBasicBlock::s_bbMapper.get(m_target) << std::endl;
}
void MipsReturn::toCode(std::ostream& os) {
    if (m_retFunc->getIrFunc()->getFuncItem()->getName() == "main") {
        os << "li $v0, 10\n"
           << "\tsyscall\n";
    } else {
        if (m_retFunc->getStackSize() != 0) {
            moveStack(false, m_retFunc->getStackSize(), os);
        }
        auto savedRegSize = m_retFunc->usedCalleeSavedRegs.size();
        if (savedRegSize) {
            int lastStack = 0;
            for (auto iter = m_retFunc->usedCalleeSavedRegs.rbegin(); iter != m_retFunc->usedCalleeSavedRegs.rend(); iter++) {
                auto& used = *iter;
                os << "\tlw " << MipsOperand::R(used) << ", " << lastStack << "($sp)\n";
                lastStack += 4;
            }
            os << "\taddu $sp, $sp, " << savedRegSize * 4 << "\n\t";
        }
        os << "jr $ra" << std::endl;
    }
}
void MipsLoad::toCode(std::ostream& os) {
    os << "lw " << m_dst << ", " << m_offset << "(" << m_addr << ")" << std::endl;
}
void MipsStore::toCode(std::ostream& os) {
    os << "sw " << m_data << ", " << m_offset << "(" << m_addr << ")" << std::endl;
}
void MipsCompare::toCode(std::ostream& os) {
    os << m_cond << " " << m_dst << ", " << m_lhs << ", " << m_rhs << std::endl;
}
void MipsCall::toCode(std::ostream& os) {
    os << "jal " << m_func->getName() << std::endl;
}
void MipsBinary::toCode(std::ostream& os) {
    os << instString() << " " << m_dst << ", " << m_lhs << ", " << m_rhs << std::endl;
}

std::pair<std::vector<MipsOperand>, std::vector<MipsOperand>> getDefUse(MipsInst* inst) {
    std::vector<MipsOperand> def;
    std::vector<MipsOperand> use;

    if (auto x = dynamic_cast<MipsBinary*>(inst)) {
        def = {x->m_dst};
        use = {x->m_lhs, x->m_rhs};
    } else if (auto x = dynamic_cast<MipsMove*>(inst)) {
        def = {x->m_dst};
        use = {x->m_rhs};
    } else if (auto x = dynamic_cast<MipsLoad*>(inst)) {
        def = {x->m_dst};
        use = {x->m_addr};
    } else if (auto x = dynamic_cast<MipsStore*>(inst)) {
        use = {x->m_data, x->m_addr};
    } else if (auto x = dynamic_cast<MipsCompare*>(inst)) {
        def = {x->m_dst};
        use = {x->m_lhs, x->m_rhs};
    } else if (auto x = dynamic_cast<MipsBranch*>(inst)) {
        use = {x->m_lhs, x->m_rhs};
    } else if (auto x = dynamic_cast<MipsCall*>(inst)) {
        // args (also caller save)
        for (int i = (int)MipsReg::a0; i < (int)MipsReg::a0 + std::min(x->m_func->getParams().size(), (size_t)4); ++i) {
            use.push_back(MipsOperand::R((MipsReg)i));
        }
        for (int i = (int)MipsReg::a0; i <= (int)MipsReg::a3; i++) {
            def.push_back(MipsOperand::R((MipsReg)i));
        }
        def.push_back(MipsOperand::R(MipsReg::sp));
        //def.push_back(MipsOperand::R(MipsReg::ip));
    } else if (auto x = dynamic_cast<MipsGlobal*>(inst)) {
        def = {x->m_dst};
    } else if (auto x = dynamic_cast<MipsString*>(inst)) {
        def = {x->m_dst};
    } else if (dynamic_cast<MipsReturn*>(inst)) {
        // ret
        use.push_back(MipsOperand::R(MipsReg::v0));
    }
    return {def, use};
}

std::pair<MipsOperand*, std::vector<MipsOperand*>> getDefUsePtr(MipsInst* inst) {
    MipsOperand* def = nullptr;
    std::vector<MipsOperand*> use;

    if (auto x = dynamic_cast<MipsBinary*>(inst)) {
        def = &x->m_dst;
        use = {&x->m_lhs, &x->m_rhs};
    } else if (auto x = dynamic_cast<MipsMove*>(inst)) {
        def = &x->m_dst;
        use = {&x->m_rhs};
    } else if (auto x = dynamic_cast<MipsLoad*>(inst)) {
        def = &x->m_dst;
        use = {&x->m_addr};
    } else if (auto x = dynamic_cast<MipsStore*>(inst)) {
        use = {&x->m_data, &x->m_addr};
    } else if (auto x = dynamic_cast<MipsCompare*>(inst)) {
        def = {&x->m_dst};
        use = {&x->m_lhs, &x->m_rhs};
    } else if (auto x = dynamic_cast<MipsBranch*>(inst)) {
        use = {&x->m_lhs, &x->m_rhs};
    } else if (dynamic_cast<MipsCall*>(inst)) {
        // intentionally blank
    } else if (auto x = dynamic_cast<MipsGlobal*>(inst)) {
        def = {&x->m_dst};
    } else if (auto x = dynamic_cast<MipsString*>(inst)) {
        def = {&x->m_dst};
    }
    return {def, use};
}