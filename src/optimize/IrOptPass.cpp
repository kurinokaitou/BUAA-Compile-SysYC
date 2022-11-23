#include <optimize/IrOptPass.h>
#include <ir/ControlFlowGraph.h>
#include <optimize/BasicBlockOptimize.h>

void memToReg(IrModule& module) {
    for (auto& func : module.m_funcs) {
        basicBlockOpt(func.get());
        computeDomInfo(func.get());
        std::unordered_map<Value*, int> allocaIds; // 把alloca映射到整数，后面有好几个vector用这个做下标
        std::vector<Value*> allocas;
        auto& basicBlock = func->getBasicBlocks();
        for (auto& bb : basicBlock) {
            auto& insts = bb->getInsts();
            for (auto& inst : insts) {
                if (auto a = dynamic_cast<AllocaInst*>(inst.get())) {
                    auto dims = getArrayItemDimensions(a->getSym());
                    if (dims.empty()) { // 局部int变量
                        allocaIds.insert({a, (int)allocaIds.size()});
                        allocas.push_back(a);
                    }
                }
            }
        }
        std::vector<std::vector<BasicBlock*>> allocaDefs(allocaIds.size());
        for (auto& bb : basicBlock) {
            auto& insts = bb->getInsts();
            for (auto& inst : insts) {
                if (auto x = dynamic_cast<StoreInst*>(inst.get())) {
                    auto it = allocaIds.find(x->getArrValue());
                    if (it != allocaIds.end()) {
                        allocaDefs[it->second].push_back(bb.get());
                    }
                }
            }
        }
        auto df = computeDf(func.get());
        // mem2reg算法阶段1：放置phi节点
        // worklist定义在循环外面，只是为了减少申请内存的次数
        std::vector<BasicBlock*> worklist;      // 用stack还是queue在这里没有本质区别
        std::unordered_map<PhiInst*, int> phis; // 记录加入的phi属于的alloca的id
        for (int id = 0; id < allocas.size(); id++) {
            func->clearAllVisitFlag();
            for (BasicBlock* bb : allocaDefs[id]) {
                worklist.push_back(bb);
            }
            while (!worklist.empty()) {
                BasicBlock* x = worklist.back();
                worklist.pop_back();
                for (BasicBlock* y : df[x]) {
                    if (!y->vis) {
                        y->vis = true;
                        auto phiInst = new PhiInst(y);
                        y->pushBackInst(phiInst);
                        phis.insert({phiInst, id});
                        worklist.push_back(y);
                    }
                }
            }
        }
        // mem2reg算法阶段2：变量重命名，即删除Load，把对Load结果的引用换成对寄存器的引用，把Store改成寄存器赋值
        std::vector<std::pair<BasicBlock*, std::vector<Value*>>> worklist2{{func->getBasicBlocks().front().get(), std::vector<Value*>(allocaIds.size(), nullptr)}};
        func->clearAllVisitFlag();
        while (!worklist2.empty()) {
            BasicBlock* bb = worklist2.back().first;
            std::vector<Value*> values = std::move(worklist2.back().second);
            worklist2.pop_back();
            if (!bb->vis) {
                bb->vis = true;
                auto& insts = bb->getInsts();
                for (auto inst = insts.begin(); inst != insts.end();) {
                    auto next = std::next(inst);
                    // 如果一个value在allocaIds中，它的实际类型必然是AllocaInst，无需再做dynamic_cast
                    auto it = allocaIds.find(inst->get());
                    if (it != allocaIds.end()) {
                        bb->removeInst(inst->get());
                    } else if (auto x = dynamic_cast<LoadInst*>(inst->get())) {
                        // 这里不能，也不用再看x->arr.value是不是AllocaInst了
                        // 不能的原因是上面的if分支会delete掉alloca；不用的原因是只要allocaIds里有，它就一定是AllocaInst
                        auto it = allocaIds.find(x->getArrValue());
                        if (it != allocaIds.end()) {
                            x->replaceAllUse(values[it->second]);
                            x->setArrValue(nullptr); // 它用到被delete的AllocaInst，已经不能再访问了
                            bb->removeInst(x);
                        }
                    } else if (auto x = dynamic_cast<StoreInst*>(inst->get())) {
                        auto it = allocaIds.find(x->getArrValue());
                        if (it != allocaIds.end()) {
                            values[it->second] = x->getDataValue();
                            x->setArrValue(nullptr);
                            bb->removeInst(x);
                        }
                    } else if (auto x = dynamic_cast<PhiInst*>(inst->get())) {
                        auto it = phis.find(x); // 也许程序中本来就存在phi，所以phis不一定包含了所有的phi
                        if (it != phis.end()) {
                            values[it->second] = x;
                        }
                    }
                    inst = next;
                }
                for (auto& x : bb->getSuccs()) {
                    if (x) {
                        worklist2.emplace_back(x, values);
                        auto& insts = x->getInsts();
                        for (auto& inst : insts) {
                            if (auto p = dynamic_cast<PhiInst*>(inst.get())) {
                                auto it = phis.find(p);
                                if (it != phis.end()) {
                                    int idx = std::find(x->getPreds().begin(), x->getPreds().end(), bb) - x->getPreds().begin(); // bb是x的哪个pred?
                                    p->getIncomingValues()[idx].set(values[it->second]);
                                }
                            } else {
                                break; // PhiInst一定是在指令序列的最前面，所以遇到第一个非PhiInst的指令就可以break了
                            }
                        }
                    }
                }
            }
        }
    }
}