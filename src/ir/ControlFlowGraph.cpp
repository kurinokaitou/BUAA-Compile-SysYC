#include <ir/ControlFlowGraph.h>
#include <cassert>

// 计算dom_level
static void dfs(BasicBlock* bb, int level) {
    bb->domLevel = level;
    for (BasicBlock* ch : bb->doms) {
        dfs(ch, level + 1);
    }
}

void computeDomInfo(IrFunc* f) {
    BasicBlock* entry = f->firstBasicBlock();
    // 计算domBy
    entry->domBy = {entry};
    std::unordered_set<BasicBlock*> all; // 全部基本块，除entry外的dom的初值
    auto& basicBlocks = f->getBasicBlocks();
    for (auto& bb : basicBlocks) {
        all.insert(bb.get());
        bb->doms.clear(); // 顺便清空doms，与计算domBy无关
    }
    for (auto bb = std::next(basicBlocks.begin()); bb != basicBlocks.end(); bb++) {
        (*bb)->domBy = all;
    }
    while (true) {
        bool changed = false;
        for (auto it = std::next(basicBlocks.begin()); it != basicBlocks.end(); it++) {
            auto& bb = *it;
            for (auto it = bb->domBy.begin(); it != bb->domBy.end();) {
                BasicBlock* x = *it;
                // 如果bb的任何一个pred的dom不包含x，那么bb的dom也不应该包含x
                if (x != bb.get() && std::any_of(bb->getPreds().begin(), bb->getPreds().end(), [x](BasicBlock* p) { return p->domBy.find(x) == p->domBy.end(); })) {
                    changed = true;
                    it = bb->domBy.erase(it);
                } else {
                    ++it;
                }
            }
        }
        if (!changed) { break; }
    }
    // 计算idom，顺便填充doms
    entry->idom = nullptr;
    for (auto it = std::next(basicBlocks.begin()); it != basicBlocks.end(); it++) {
        auto& bb = *it;
        for (BasicBlock* d : bb->domBy) {
            // 已知d dom bb，若d != bb，则d strictly dom bb
            // 若还有：d不strictly dom任何strictly dom bb的节点，则d idom bb
            if (d != bb.get() && std::all_of(bb->domBy.begin(), bb->domBy.end(), [d, &bb](BasicBlock* x) {
                    return x == bb.get() || x == d || x->domBy.find(d) == x->domBy.end();
                })) {
                bb->idom = d; // 若实现正确，这里恰好会执行一次(即使没有break)
                d->doms.push_back(bb.get());
                break;
            }
        }
    }
    dfs(entry, 0);
}

// 在dom tree上后序遍历，识别所有循环
// 在所有递归调用中用的是同一个worklist，这只是为了减少内存申请，它们之间没有任何关系
static void collectLoops(LoopInfo& info, std::vector<BasicBlock*>& worklist, BasicBlock* header) {
    for (BasicBlock* s : header->doms) {
        if (s) collectLoops(info, worklist, s);
    }
    assert(worklist.empty());
    for (BasicBlock* p : header->getPreds()) {         // 存在p到header的边
        if (p->domBy.find(header) != p->domBy.end()) { // ...且header支配p，这是回边
            worklist.push_back(p);
        }
    }
    if (!worklist.empty()) {
        Loop* l = new Loop(header);
        while (!worklist.empty()) {
            BasicBlock* pred = worklist.back();
            worklist.pop_back();
            auto pair = info.bbLoop.insert({pred, l});
            auto& it = pair.first;
            auto& inserted = pair.second;
            if (inserted) {
                // 插入成功意味着pred原先不属于任何loop，现在它属于这个loop了
                if (pred != header) {
                    worklist.insert(worklist.end(), pred->getPreds().begin(), pred->getPreds().end());
                }
            } else {
                // 这是一个已经发现的loop
                Loop* sub = it->second;
                while (Loop* p = sub->parent) sub = p; // 找到已发现的最外层的loop
                if (sub != l) {
                    sub->parent = l;
                    // 只需考虑sub的header的pred，因为根据循环的性质，循环中其他bb的pred必然都在循环内
                    for (BasicBlock* pred : sub->header()->getPreds()) {
                        auto it = info.bbLoop.find(pred);
                        if (it == info.bbLoop.end() || it->second != sub) {
                            worklist.push_back(pred);
                        }
                    }
                }
            }
        }
    }
}

// 填充Loop::bbs, sub_loops, LoopInfo::top_level
// todo: llvm是依据bb的后序遍历来填的，这个顺序不会影响任何内容的存在与否，只会影响内容的顺序，那么这个顺序重要吗？
static void populate(LoopInfo& info, BasicBlock* bb) {
    if (bb->vis) return;
    bb->vis = true;
    for (BasicBlock* s : bb->getSuccs()) {
        if (s) populate(info, s);
    }
    auto it = info.bbLoop.find(bb);
    Loop* sub = it == info.bbLoop.end() ? nullptr : it->second;
    if (sub && sub->header() == bb) {
        (sub->parent ? sub->parent->sub_loops : info.topLevel).push_back(sub);
        std::reverse(sub->bbs.begin() + 1, sub->bbs.end());
        std::reverse(sub->sub_loops.begin(), sub->sub_loops.end());
        sub = sub->parent;
    }
    for (; sub; sub = sub->parent)
        sub->bbs.push_back(bb);
}

LoopInfo computeLoopInfo(IrFunc* f) {
    computeDomInfo(f);
    LoopInfo info;
    std::vector<BasicBlock*> worklist;
    auto head = f->firstBasicBlock();
    collectLoops(info, worklist, head);
    f->clearAllVisitFlag();
    populate(info, head);
    return info;
}

static void dfs(std::vector<BasicBlock*>& po, BasicBlock* bb) {
    if (!bb->vis) {
        bb->vis = true;
        for (BasicBlock* x : bb->getSuccs()) {
            if (x) dfs(po, x);
        }
        po.push_back(bb);
    }
}

std::vector<BasicBlock*> computeRpo(IrFunc* f) {
    std::vector<BasicBlock*> ret;
    f->clearAllVisitFlag();
    auto head = f->firstBasicBlock();
    dfs(ret, head);
    std::reverse(ret.begin(), ret.end());
    return ret;
}

std::unordered_map<BasicBlock*, std::unordered_set<BasicBlock*>> computeDf(IrFunc* f) {
    std::unordered_map<BasicBlock*, std::unordered_set<BasicBlock*>> df;
    auto& basicBlocks = f->getBasicBlocks();
    for (auto& from : basicBlocks) {
        for (BasicBlock* to : from->getSuccs()) {
            if (to) { // 枚举所有边(from, to)
                BasicBlock* x = from.get();
                while (x == to || to->domBy.find(x) == to->domBy.end()) { // while x不strictly dom to
                    df[x].insert(to);
                    x = x->idom;
                }
            }
        }
    }
    return df;
}