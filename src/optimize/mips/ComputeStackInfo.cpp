#include <optimize/mips/ComputeStackInfo.h>
#include <mips/MipsCode.h>

void computeStackInfo(MipsModule& module) {
    for (auto& f : module.m_funcs) {
        f->usedCalleeSavedRegs.insert(MipsReg::ra);

        for (auto& bb : f->getMipsBasicBlocks()) {
            auto& insts = bb->getMipsInsts();
            for (auto& inst : insts) {
                auto def = std::get<0>(getDefUse(inst.get()));
                for (const auto& reg : def) {
                    if ((int)MipsReg::t0 <= reg.value && reg.value <= (int)MipsReg::t9) {
                        f->usedCalleeSavedRegs.insert((MipsReg)reg.value);
                    }
                }
            }
        }

        // fixup arg access
        int savedRegs = f->usedCalleeSavedRegs.size();

        for (auto& spArgInst : f->spArgFixup) {
            if (auto x = dynamic_cast<MipsLoad*>(spArgInst)) {
                x->setOffset(x->getOffset() + f->getStackSize() + 4 * savedRegs);
            }
        }
    }
}
