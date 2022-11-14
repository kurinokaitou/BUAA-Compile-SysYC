#include <mips/MipsCode.h>
IndexMapper<MipsBasicBlock> MipsBasicBlock::s_bbMapper;

void MipsModule::toCode(std::ostream& os) {
    int size = 0;
    os << ".data" << std::endl;
    for (auto& glob : m_globs) {
        auto globItem = glob->getGlobalItem();
        auto values = getItemValues(globItem);
        os << globItem->getName() << ":" << std::endl;
        bool hasInit = globItem->hasInit();
        for (auto& value : values) {
            os << "\t.word " << (hasInit ? value : 0) << "\n";
        }
    }
    os << ".text" << std::endl;
    for (auto& func : m_funcs) {
        func->toCode(os);
    }
    os << std::endl;
}

void MipsFunc::toCode(std::ostream& os) {
    os << m_irFunc->getFuncItem()->getName() << ":" << std::endl;
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
    os << "jr %ra" << std::endl;
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