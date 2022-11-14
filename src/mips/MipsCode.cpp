#include <mips/MipsCode.h>
IndexMapper<MipsBasicBlock> MipsBasicBlock::s_bbMapper;

void MipsModule::toCode(std::ostream& os) {
    int size = 0;
    os << ".data" << std::endl;
    for (auto& glob : m_globs) {
        auto globItem = glob->getGlobalItem();
        auto values = getItemValues(globItem);
        os << globItem->getName() << ":" << std::endl;
        for (auto& value : values) {
            os << "\t.word " << value << "\n";
        }
        os << std::endl;
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
        inst->toCode(os);
    }
    os << std::endl;
}

void MipsGlobal::toCode(std::ostream& os) {
    os << "# load global variable addr" << m_sym->getName() << std::endl;
}

void MipsMove::toCode(std::ostream& os) {
    os << "move" << std::endl;
}
void MipsBranch::toCode(std::ostream& os) {
    os << "beq" << std::endl;
}
void MipsJump::toCode(std::ostream& os) {
    os << "j" << std::endl;
}
void MipsReturn::toCode(std::ostream& os) {
    os << "jr" << std::endl;
}
void MipsLoad::toCode(std::ostream& os) {
    os << "lw" << std::endl;
}
void MipsStore::toCode(std::ostream& os) {
    os << "sw" << std::endl;
}
void MipsCompare::toCode(std::ostream& os) {
    os << "cmp" << std::endl;
}
void MipsCall::toCode(std::ostream& os) {
    os << "jal" << std::endl;
}
void MipsBinary::toCode(std::ostream& os) {
    os << "binary" << std::endl;
}