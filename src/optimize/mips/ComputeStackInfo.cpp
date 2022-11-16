#include <optimize/mips/ComputeStackInfo.h>
#include <mips/MipsCode.h>

void computeStackInfo(MipsModule& module) {
    for (auto& f : module.m_funcs) {
        f->usedCalleeSavedRegs.insert(MipsReg::ra);
        // auto argsNum = f->getIrFunc()->getFuncItem()->getParams().size();
        // for (int i = 0; i < argsNum; i++) {
        //     f->usedCalleeSavedRegs.insert({MipsReg((int)MipsReg::a0 + i)});
        // }
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

        // // fixup arg access
        // // r4-r11, lr
        // int savedRegs = f->usedCalleeSavedRegs.size();

        // for (auto& spArgInst : f->spArgFixup) {
        //     // mv r0, imm
        //     // ldr, [sp, r0]
        //     if (auto x = dynamic_cast<MipsMove*>(spArgInst)) {
        //         x->getRhs().value += f->getStackSize() + 4 * savedRegs;
        //     }
        // }
    }
}
