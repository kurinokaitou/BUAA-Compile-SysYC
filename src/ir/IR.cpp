#include <ir/IR.h>
std::array<BasicBlock*, 2> BasicBlock::getSuccs() {
    auto& lastInst = m_insts.back();
    if (auto branchInst = dynamic_cast<BranchInst*>(lastInst.get())) {
        return {branchInst->m_left, branchInst->m_right};
    } else if (auto jumpInst = dynamic_cast<JumpInst*>(lastInst.get())) {
        return {jumpInst->m_next, nullptr};
    } else if (auto returnInst = dynamic_cast<ReturnInst*>(lastInst.get())) {
        return {nullptr, nullptr};
    } else {
        // Log error;
        return {nullptr, nullptr};
    }
}