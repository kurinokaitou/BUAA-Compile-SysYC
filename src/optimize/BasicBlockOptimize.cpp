#include <optimize/BasicBlockOptimize.h>
#include <ir/IR.h>

static void dfs(BasicBlock* bb) {
    if (!bb->vis) {
        bb->vis = true;
        for (BasicBlock* x : bb->getSuccs()) {
            if (x) dfs(x);
        }
    }
}
// 如果发生了将入度为1的bb的phi直接替换成这个值的优化，则返回true，gvn_gcm需要这个信息，因为这可能产生新的优化机会
// 如果不这样做，最终交给后端的ir可能包含常量间的二元运算，这是后端不允许的
void basicBlockOpt(IrFunc* func) {
    bool changed;
    do {
        changed = false;
        // 这个循环本来只是为了消除if (常数)，没有必要放在do while里的，但是在这里消除if (x) br a else br a也比较方便
        // 后面这种情形会被下面的循环引入，所以这个循环也放在do while里
        for (auto& bb : func->getBasicBlocks()) {
            auto& back = bb->getInsts().back();
            if (auto x = dynamic_cast<BranchInst*>(back.get())) {
                BasicBlock* deleted = nullptr;
                if (auto cond = dynamic_cast<ConstValue*>(x->getCondValue())) {
                    bb->pushBackInst(new JumpInst(cond->getImm() ? x->getTrueBasicBlock() : x->getFalseBasicBlock()));
                    deleted = cond->getImm() ? x->getFalseBasicBlock() : x->getTrueBasicBlock();
                } else if (x->getTrueBasicBlock() == x->getFalseBasicBlock()) { // 可能被消除以jump结尾的空基本块引入
                    bb->pushBackInst(new JumpInst(x->getTrueBasicBlock()));
                    deleted = x->getFalseBasicBlock();
                    changed = true; // 可能引入新的以jump结尾的空基本块
                }
                if (deleted) {
                    bb->removeInst(x);
                    int idx = std::find(deleted->getPreds().begin(), deleted->getPreds().end(), bb.get()) - deleted->getPreds().begin();
                    deleted->getPreds().erase(deleted->getPreds().begin() + idx);
                    for (auto& i : deleted->getInsts()) {
                        if (auto phi = dynamic_cast<PhiInst*>(i.get()))
                            phi->getIncomingValues().erase(phi->getIncomingValues().begin() + idx);
                        else {
                            break;
                        }
                    }
                }
            }
        }
        // 这个循环消除以jump结尾的空基本块
        // 简单起见不考虑第一个bb，因为第一个bb是entry，把它删了需要把新的entry移动到第一个来
        auto& basicBlocks = func->getBasicBlocks();
        for (auto bb = basicBlocks.begin(); bb != basicBlocks.end();) {
            auto next = std::next(bb);
            // 要求target != bb，避免去掉空的死循环
            auto& insts = (*bb)->getInsts();
            auto x = dynamic_cast<JumpInst*>(insts.back().get());
            if (x && x->getNextBasicBlock() != (*bb).get() && insts.front() == insts.back()) {
                BasicBlock* target = x->getNextBasicBlock();
                // 如果存在一个pred，它以BranchInst结尾，且left或right已经为target，且target中存在phi，则不能把另一个也变成bb
                // 例如 bb1: { b = a + 1; if (x) br bb2 else br bb3 } bb2: { br bb3; } bb3: { c = phi [a, bb1] [b bb2] }
                // 这时bb2起到了一个区分phi来源的作用
                bool flag = true;
                if (dynamic_cast<PhiInst*>(target->getInsts().front().get())) {
                    for (BasicBlock* p : (*bb)->getPreds()) {
                        if (auto br = dynamic_cast<BranchInst*>(p->getInsts().back().get())) {
                            if (br->getTrueBasicBlock() == target || br->getFalseBasicBlock() == target) {
                                flag = false;
                                break;
                            }
                        }
                    }
                }
                if (flag) {
                    int idx = std::find(target->getPreds().begin(), target->getPreds().end(), bb->get()) - target->getPreds().begin();
                    target->getPreds().erase(target->getPreds().begin() + idx);
                    for (BasicBlock* p : (*bb)->getPreds()) {
                        auto succ = p->getSuccsRef();
                        **std::find_if(succ.begin(), succ.end(), [bb](BasicBlock** y) { return *y == (*bb).get(); }) = target;
                        target->getPreds().push_back(p);
                    }
                    int predSize = (*bb)->getPreds().size();
                    for (auto& i : target->getInsts()) {
                        if (auto phi = dynamic_cast<PhiInst*>(i.get())) {
                            Value* v = phi->getIncomingValues()[idx].value;
                            phi->getIncomingValues().erase(phi->getIncomingValues().begin() + idx);
                            for (int j = 0; j < predSize; ++j) {
                                phi->getIncomingValues().emplace_back(v, phi);
                            }
                        } else {
                            break;
                        }
                    }
                    func->removeBasicBlock((*bb).get());
                    changed = true;
                }
            }
            bb = next;
        }
    } while (changed);

    func->clearAllVisitFlag();
    dfs(func->getBasicBlocks().front().get());
    auto& basicBlock = func->getBasicBlocks();
    // 不可达的bb仍然可能有指向可达的bb的边，需要删掉目标bb中的pred和phi中的这一项
    for (auto& bb : basicBlock) {
        if (!bb->vis) {
            for (BasicBlock* s : bb->getSuccs()) {
                if (s && s->vis) {
                    int idx = std::find(s->getPreds().begin(), s->getPreds().end(), bb.get()) - s->getPreds().begin();
                    s->getPreds().erase(s->getPreds().begin() + idx);
                    auto& insts = s->getInsts();
                    for (auto& i : insts) {
                        if (auto x = dynamic_cast<PhiInst*>(i.get()))
                            x->getIncomingValues().erase(x->getIncomingValues().begin() + idx);
                        else {
                            break;
                        }
                    }
                }
            }
        }
    }
    for (auto it = basicBlock.begin(); it != basicBlock.end();) {
        auto bb = (*it).get();
        auto next = std::next(it);
        if (!bb->vis) {
            func->removeBasicBlock(bb);
        }
        it = next;
    }

    for (auto& bb : basicBlock) {
        if (bb->getPreds().size() == 1) {
            auto& insts = bb->getInsts();
            for (auto it = insts.begin(); it != insts.end();) {
                auto next = std::next(it);
                auto i = (*it).get();
                if (auto x = dynamic_cast<PhiInst*>(i)) {
                    assert(x->getIncomingValues().size() == 1);
                    x->replaceAllUse(x->getIncomingValues()[0].value);
                    bb->removeInst(x);
                } else {
                    break;
                }
                it = next;
            }
        }
    }
}