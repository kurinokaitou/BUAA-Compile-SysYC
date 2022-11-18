#include <optimize/MipsOptPass.h>

#include <algorithm>
#include <cmath>
#include <map>
#include <set>

#include <mips/MipsCode.h>
#include <ir/ControlFlowGraph.h>

void livenessAnalysis(MipsFunc* f) {
    // calculate LiveUse and Def sets for each bb
    // each elements is a virtual register or precolored register
    for (auto& bb : f->m_basicBlocks) {
        bb->liveuse.clear();
        bb->def.clear();
        for (auto& inst : bb->m_insts) {
            auto pair = getDefUse(inst.get());
            auto def = pair.first;
            auto use = pair.second;
            // liveuse
            for (auto& u : use) {
                if (u.needsColor() && bb->def.find(u) == bb->def.end()) {
                    bb->liveuse.insert(u);
                }
            }
            // def
            for (auto& d : def) {
                if (d.needsColor() && bb->liveuse.find(d) == bb->liveuse.end()) {
                    bb->def.insert(d);
                }
            }
        }
        // initial values
        bb->livein = bb->liveuse;
        bb->liveout.clear();
    }

    // calculate LiveIn and LiveOut for each bb
    bool changed = true;
    while (changed) {
        changed = false;
        for (auto& bb : f->m_basicBlocks) {
            std::set<MipsOperand> newOut;
            for (auto& succ : bb->getSuccs()) {
                if (succ) {
                    newOut.insert(succ->livein.begin(), succ->livein.end());
                }
            }

            if (newOut != bb->liveout) {
                changed = true;
                bb->liveout = newOut;
                std::set<MipsOperand> newIn = bb->liveuse;
                for (auto& e : bb->liveout) {
                    if (bb->def.find(e) == bb->def.end()) {
                        newIn.insert(e);
                    }
                }

                bb->livein = newIn;
            }
        }
    };
}

// iterated register coalescing
void allocateRegister(MipsModule& module) {
    for (auto& f : module.m_funcs) {
        auto loopInfo = computeLoopInfo(f->getIrFunc());
        bool done = false;
        while (!done) {
            livenessAnalysis(f.get());
            // interference graph
            // each node is a MipsOperand
            // can only Precolored or Virtual
            // adjacent list
            std::map<MipsOperand, std::set<MipsOperand>> adjList;
            // adjacent set
            std::set<std::pair<MipsOperand, MipsOperand>> adjSet;
            // other variables in the paper
            std::map<MipsOperand, int> degree;
            std::map<MipsOperand, MipsOperand> alias;
            std::map<MipsOperand, std::set<MipsMove*, MipsMoveCompare>> moveList;
            std::set<MipsOperand> simplifyWorklist;
            std::set<MipsOperand> freezeWorklist;
            std::set<MipsOperand> spillWorklist;
            std::set<MipsOperand> spilledNodes;
            std::set<MipsOperand> coalescedNodes;
            std::vector<MipsOperand> coloredNodes;
            std::vector<MipsOperand> selectStack;
            std::set<MipsMove*, MipsMoveCompare> coalescedMoves;
            std::set<MipsMove*, MipsMoveCompare> constrainedMoves;
            std::set<MipsMove*, MipsMoveCompare> frozenMoves;
            std::set<MipsMove*, MipsMoveCompare> worklistMoves;
            std::set<MipsMove*, MipsMoveCompare> activeMoves;
            // for heuristic
            std::map<MipsOperand, int> loopCnt;
            auto& basicBlocks = f->getMipsBasicBlocks();
            // allocatable registers: t0 to t9
            constexpr int k = (int)MipsReg::t9 - (int)MipsReg::t0 + 1; //+ 1;
            // init degree for pre colored nodes
            for (int i = (int)MipsReg::v0; i <= (int)MipsReg::a3; i++) {
                auto op = MipsOperand::R((MipsReg)i);
                // very large
                degree[op] = 0x40000000;
            }
            degree[MipsOperand::R(MipsReg::sp)] = 0x40000000;
            degree[MipsOperand::R(MipsReg::ra)] = 0x40000000;

            // procedure AddEdge(u, v)
            auto addEdge = [&](MipsOperand u, MipsOperand v) {
                if (adjSet.find({u, v}) == adjSet.end() && u != v) {
                    adjSet.insert({u, v});
                    adjSet.insert({v, u});
                    if (!u.isPrecolored()) {
                        adjList[u].insert(v);
                        degree[u]++;
                    }
                    if (!v.isPrecolored()) {
                        adjList[v].insert(u);
                        degree[v]++;
                    }
                }
            };

            // procedure Build()
            auto build = [&]() {
                // build interference graph
                for (auto iter = basicBlocks.rbegin(); iter != basicBlocks.rend(); iter++) {
                    auto bb = (*iter).get();
                    // calculate live set before each instruction
                    auto live = bb->liveout;
                    auto& insts = bb->getMipsInsts();
                    for (auto iter = insts.rbegin(); iter != insts.rend(); iter++) {
                        auto inst = (*iter).get();
                        auto pair = getDefUse(inst);
                        auto& def = pair.first;
                        auto& use = pair.second;
                        if (auto x = dynamic_cast<MipsMove*>(inst)) {
                            if (x->getDst().needsColor() && x->getRhs().needsColor()) {
                                live.erase(x->getRhs());
                                moveList[x->getRhs()].insert(x);
                                moveList[x->getDst()].insert(x);
                                worklistMoves.insert(x);
                            }
                        }

                        for (auto& d : def) {
                            if (d.needsColor()) {
                                live.insert(d);
                            }
                        }

                        for (auto& d : def) {
                            if (d.needsColor()) {
                                for (auto& l : live) {
                                    addEdge(l, d);
                                }
                            }
                        }

                        for (auto& d : def) {
                            if (d.needsColor()) {
                                live.erase(d);
                                loopCnt[d] += loopInfo.depthOf(bb->getIrBasicBlock());
                            }
                        }

                        for (auto& u : use) {
                            if (u.needsColor()) {
                                live.insert(u);
                                loopCnt[u] += loopInfo.depthOf(bb->getIrBasicBlock());
                            }
                        }
                    }
                }
            };

            auto adjacent = [&](MipsOperand n) {
                std::set<MipsOperand> res = adjList[n];
                for (auto it = res.begin(); it != res.end();) {
                    if (std::find(selectStack.begin(), selectStack.end(), *it) == selectStack.end()
                        && std::find(coalescedNodes.begin(), coalescedNodes.end(), *it) == coalescedNodes.end()) {
                        it++;
                    } else {
                        it = res.erase(it);
                    }
                }
                return res;
            };

            auto nodeMoves = [&](MipsOperand n) {
                std::set<MipsMove*, MipsMoveCompare> res = moveList[n];
                for (auto it = res.begin(); it != res.end();) {
                    if (activeMoves.find(*it) == activeMoves.end() && worklistMoves.find(*it) == worklistMoves.end()) {
                        it = res.erase(it);
                    } else {
                        it++;
                    }
                }
                return res;
            };

            auto moveRelated = [&](MipsOperand n) { return !nodeMoves(n).empty(); };

            auto makeWorklist = [&]() {
                for (int i = 0; i < f->getVirtualMax(); i++) {
                    // initial
                    auto vreg = MipsOperand::V(i);
                    if (degree[vreg] >= k) {
                        spillWorklist.insert(vreg);
                    } else if (moveRelated(vreg)) {
                        freezeWorklist.insert(vreg);
                    } else {
                        simplifyWorklist.insert(vreg);
                    }
                }
            };

            // EnableMoves({m} u Adjacent(m))
            auto enableMoves = [&](MipsOperand n) {
                for (auto m : nodeMoves(n)) {
                    if (activeMoves.find(m) != activeMoves.end()) {
                        activeMoves.erase(m);
                        worklistMoves.insert(m);
                    }
                }

                for (auto a : adjacent(n)) {
                    for (auto m : nodeMoves(a)) {
                        if (activeMoves.find(m) != activeMoves.end()) {
                            activeMoves.erase(m);
                            worklistMoves.insert(m);
                        }
                    }
                }
            };

            auto decrementDegree = [&](MipsOperand m) {
                auto d = degree[m];
                degree[m] = d - 1;
                if (d == k) {
                    enableMoves(m);
                    spillWorklist.insert(m);
                    if (moveRelated(m)) {
                        freezeWorklist.insert(m);
                    } else {
                        simplifyWorklist.insert(m);
                    }
                }
            };

            auto simplify = [&]() {
                auto it = simplifyWorklist.begin();
                auto n = *it;
                simplifyWorklist.erase(it);
                selectStack.push_back(n);
                for (auto& m : adjacent(n)) {
                    decrementDegree(m);
                }
            };

            // procedure GetAlias(n)
            auto getAlias = [&](MipsOperand n) -> MipsOperand {
                while (std::find(coalescedNodes.begin(), coalescedNodes.end(), n) != coalescedNodes.end()) {
                    n = alias[n];
                }
                return n;
            };

            // procedure AddWorkList(n)
            auto addWorkList = [&](MipsOperand u) {
                if (!u.isPrecolored() && !moveRelated(u) && degree[u] < k) {
                    freezeWorklist.erase(u);
                    simplifyWorklist.insert(u);
                }
            };

            auto ok = [&](MipsOperand t, MipsOperand r) {
                return degree[t] < k || t.isPrecolored() || adjSet.find({t, r}) != adjSet.end();
            };

            auto adjOk = [&](MipsOperand v, MipsOperand u) {
                for (auto t : adjacent(v)) {
                    if (!ok(t, u)) {
                        return false;
                    }
                }
                return true;
            };

            // procedure Combine(u, v)
            auto combine = [&](MipsOperand u, MipsOperand v) {
                auto it = freezeWorklist.find(v);
                if (it != freezeWorklist.end()) {
                    freezeWorklist.erase(it);
                } else {
                    spillWorklist.erase(v);
                }

                coalescedNodes.insert(v);
                alias[v] = u;
                // NOTE: nodeMoves should be moveList
                auto& m = moveList[u];
                for (auto n : moveList[v]) {
                    m.insert(n);
                }
                for (auto t : adjacent(v)) {
                    addEdge(t, u);
                    decrementDegree(t);
                }

                if (degree[u] >= k && freezeWorklist.find(u) != freezeWorklist.end()) {
                    freezeWorklist.erase(u);
                    spillWorklist.insert(u);
                }
            };

            auto conservative = [&](std::set<MipsOperand> adjU, std::set<MipsOperand> adjV) {
                int count = 0;
                // set union
                for (auto n : adjV) {
                    adjU.insert(n);
                }
                for (auto n : adjU) {
                    if (degree[n] >= k) {
                        count++;
                    }
                }

                return count < k;
            };

            // procedure Coalesce()
            auto coalesce = [&]() {
                auto m = *worklistMoves.begin();
                auto u = getAlias(m->getDst());
                auto v = getAlias(m->getRhs());
                // swap when needed
                if (v.isPrecolored()) {
                    auto temp = u;
                    u = v;
                    v = temp;
                }
                worklistMoves.erase(m);

                if (u == v) {
                    coalescedMoves.insert(m);
                    addWorkList(u);
                } else if (v.isPrecolored() || adjSet.find({u, v}) != adjSet.end()) {
                    constrainedMoves.insert(m);
                    addWorkList(u);
                    addWorkList(v);
                } else if ((u.isPrecolored() && adjOk(v, u)) || (!u.isPrecolored() && conservative(adjacent(u), adjacent(v)))) {
                    coalescedMoves.insert(m);
                    combine(u, v);
                    addWorkList(u);
                } else {
                    activeMoves.insert(m);
                }
            };
            // procedure FreezeMoves(u)
            auto freezeMoves = [&](MipsOperand u) {
                for (auto m : nodeMoves(u)) {
                    if (activeMoves.find(m) != activeMoves.end()) {
                        activeMoves.erase(m);
                    } else {
                        worklistMoves.erase(m);
                    }
                    frozenMoves.insert(m);

                    auto v = m->getDst() == u ? m->getRhs() : m->getDst();
                    if (!moveRelated(v) && degree[v] < k) {
                        freezeWorklist.erase(v);
                        simplifyWorklist.insert(v);
                    }
                }
            };

            // procedure Freeze()
            auto freeze = [&]() {
                auto u = *freezeWorklist.begin();
                freezeWorklist.erase(u);
                simplifyWorklist.insert(u);
                freezeMoves(u);
            };

            // procedure SelectSpill()
            auto selectSpill = [&]() {
                MipsOperand m{};
                // select node with max degree (heuristic)
                m = *std::max_element(spillWorklist.begin(), spillWorklist.end(), [&](MipsOperand a, MipsOperand b) {
                    return float(degree[a]) / pow(2, loopCnt[a]) < float(degree[b]) / pow(2, loopCnt[b]);
                });
                simplifyWorklist.insert(m);
                freezeMoves(m);
                spillWorklist.erase(m);
            };

            // procedure AssignColors()
            auto assignColors = [&]() {
                // mapping from virtual register to its allocated register
                std::map<MipsOperand, MipsOperand> colored;
                while (!selectStack.empty()) {
                    auto n = selectStack.back();
                    selectStack.pop_back();
                    std::set<int> okColors;
                    for (int i = (int)MipsReg::t0; i <= (int)MipsReg::t9; i++) {
                        okColors.insert(i);
                    }

                    for (auto w : adjList[n]) {
                        auto a = getAlias(w);
                        if (a.state == MipsOperand::State::Allocated || a.isPrecolored()) {
                            okColors.erase(a.value);
                        } else if (a.state == MipsOperand::State::Virtual) {
                            auto it = colored.find(a);
                            if (it != colored.end()) {
                                okColors.erase(it->second.value);
                            }
                        }
                    }

                    if (okColors.empty()) {
                        spilledNodes.insert(n);
                    } else {
                        auto color = *okColors.begin();
                        colored[n] = MipsOperand{MipsOperand::State::Allocated, color};
                    }
                }

                // for testing, might not needed
                if (!spilledNodes.empty()) {
                    return;
                }

                for (auto n : coalescedNodes) {
                    auto a = getAlias(n);
                    if (a.isPrecolored()) {
                        colored[n] = a;
                    } else {
                        colored[n] = colored[a];
                    }
                }

                // replace usage of virtual registers
                for (auto& bb : basicBlocks) {
                    auto& insts = bb->getMipsInsts();
                    for (auto& inst : insts) {
                        auto pair = getDefUsePtr(inst.get());
                        auto& def = pair.first;
                        auto& use = pair.second;
                        if (def && colored.find(*def) != colored.end()) {
                            *def = colored[*def];
                        }

                        for (auto& u : use) {
                            if (u && colored.find(*u) != colored.end()) {
                                *u = colored[*u];
                            }
                        }
                    }
                }
            };

            build();
            makeWorklist();
            do {
                if (!simplifyWorklist.empty()) {
                    simplify();
                }
                if (!worklistMoves.empty()) {
                    coalesce();
                }
                if (!freezeWorklist.empty()) {
                    freeze();
                }
                if (!spillWorklist.empty()) {
                    selectSpill();
                }
            } while (!simplifyWorklist.empty() || !worklistMoves.empty() || !freezeWorklist.empty() || !spillWorklist.empty());
            assignColors();
            if (spilledNodes.empty()) {
                done = true;
            } else {
                for (auto& n : spilledNodes) {
                    // allocate on stack
                    for (auto& bb : basicBlocks) {
                        auto offset = f->getStackSize();

                        // generate a MipsLoad before first use, and a MipsStore after last def
                        MipsInst* firstUse = nullptr;
                        MipsInst* lastDef = nullptr;
                        int vreg = -1;
                        auto checkPoint = [&]() {
                            if (firstUse) {
                                auto bb = firstUse->getAtBlock();
                                bb->insertBeforeInst(firstUse, new MipsLoad(MipsOperand::V(vreg), MipsOperand::R(MipsReg::sp), offset));
                                firstUse = nullptr;
                            }
                            if (lastDef) {
                                auto bb = lastDef->getAtBlock();
                                bb->insertAfterInst(lastDef, new MipsStore(MipsOperand::V(vreg), MipsOperand::R(MipsReg::sp), offset));
                                lastDef = nullptr;
                            }
                            vreg = -1;
                        };

                        int i = 0;
                        auto& insts = bb->getMipsInsts();
                        for (auto& origInst : insts) {
                            auto pair = getDefUsePtr(origInst.get());
                            auto& def = pair.first;
                            auto& use = pair.second;
                            if (def && *def == n) {
                                // store
                                if (vreg == -1) {
                                    vreg = f->getVirtualMax();
                                    f->setVirtualMax(vreg + 1);
                                }
                                def->value = vreg;
                                lastDef = origInst.get();
                            }

                            for (auto& u : use) {
                                if (*u == n) {
                                    // load
                                    if (vreg == -1) {
                                        vreg = f->getVirtualMax();
                                        f->setVirtualMax(vreg + 1);
                                    }
                                    u->value = vreg;
                                    if (!firstUse && !lastDef) {
                                        firstUse = origInst.get();
                                    }
                                }
                            }

                            if (i++ > 30) {
                                // don't span vreg for too long
                                checkPoint();
                            }
                        }
                        checkPoint();
                    }
                    f->addStackSize(4); // increase stack size
                }
                done = false;
            }
        }
    }
}

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