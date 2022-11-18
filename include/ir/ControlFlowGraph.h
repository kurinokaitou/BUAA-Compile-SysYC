#ifndef CALL_FUNC_GRAPH_H
#define CALL_FUNC_GRAPH_H

#include <unordered_map>
#include <unordered_set>

#include "IR.h"

struct Loop {
    Loop* parent;
    std::vector<Loop*> sub_loops;
    // bbs[0]是loop header
    std::vector<BasicBlock*> bbs;

    explicit Loop(BasicBlock* header) :
        parent(nullptr), bbs{header} {}

    BasicBlock* header() { return bbs[0]; }

    // 对于顶层的循环返回1
    int depth() {
        int ret = 0;
        for (Loop* x = this; x; x = x->parent) ++ret;
        return ret;
    }

    void getDeepestLoops(std::vector<Loop*>& deepest) {
        if (sub_loops.empty())
            deepest.push_back(this);
        else
            for (Loop* x : sub_loops) x->getDeepestLoops(deepest);
    }
};

struct LoopInfo {
    // 返回bb所处的最深的循环
    std::unordered_map<BasicBlock*, Loop*> bbLoop;
    std::vector<Loop*> topLevel;

    // 若bb不在任何循环中，返回0
    int depthOf(BasicBlock* bb) {
        auto it = bbLoop.find(bb);
        return it == bbLoop.end() ? 0 : it->second->depth();
    }

    std::vector<Loop*> deepest_loops() {
        std::vector<Loop*> deepest;
        for (Loop* l : topLevel) l->getDeepestLoops(deepest);
        return deepest;
    }
};

void computeDomInfo(IrFunc* f);

// 这里假定dom树已经造好了
LoopInfo computeLoopInfo(IrFunc* f);

// 计算bb的rpo序
std::vector<BasicBlock*> computeRpo(IrFunc* f);

// 计算支配边界DF，这里用一个map来存每个bb的df，其实是很随意的选择，把它放在BasicBlock里面也不是不行
std::unordered_map<BasicBlock*, std::unordered_set<BasicBlock*>> computeDf(IrFunc* f);

#endif