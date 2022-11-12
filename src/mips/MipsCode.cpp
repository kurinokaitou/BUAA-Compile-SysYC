#include <mips/MipsCode.h>
void MipsModule::toCode(std::ostream& os) {
    int size = 0;
    for (auto& func : m_funcs) {
        func->toCode(os);
    }
    os << std::endl;
}

void MipsFunc::toCode(std::ostream& os) {
    for (auto& mbb : m_basicBlocks) {
        mbb->toCode(os);
    }
    os << std::endl;
}

void MipsBasicBlock::toCode(std::ostream& os) {
    for (auto& inst : m_insts) {
        inst->toCode(os);
    }
    os << std::endl;
}

void MipsGlobal::toCode(std::ostream& os) {
    os << "global" << std::endl;
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