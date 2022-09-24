#include <codegen/Vistitor.h>

Visitor::Visitor(std::shared_ptr<VNodeBase> astRoot, SymbolTable& table) :
    m_astRoot(astRoot), m_table(table) {
}

void Visitor::visit() {
    if (m_astRoot->getType() == VType::VN) {
        compUnit(m_astRoot);
    }
}

bool Visitor::expect(std::shared_ptr<VNodeBase> node, VNodeEnum nodeEnum) {
    if (node->getType() == VType::VN) {
        return node->getNodeEnum() == nodeEnum;
    } else {
        return false;
    }
}

bool Visitor::expect(std::shared_ptr<VNodeBase> node, SymbolEnum symbolEnum) {
    if (node->getType() == VType::VT) {
        return node->getSymbol() == symbolEnum;
    } else {
        return false;
    }
}

void Visitor::compUnit(std::shared_ptr<VNodeBase> node) {
    // TODO: codegen global scope
    while (expect(*node->getCurrChildIter(), VNodeEnum::DECL)) {
        decl(*node->getCurrChildIter());
        if (node->nextChild()) break;
    }
    while (expect(*node->getCurrChildIter(), VNodeEnum::FUNCDEF)) {
        funcDef(*node->getCurrChildIter());
        if (node->nextChild()) break;
    }
    if (expect(*node->getCurrChildIter(), VNodeEnum::MAINFUNCDEF)) {
        mainFuncDef(*node->getCurrChildIter());
    }

    m_table.popScope();
}

void Visitor::funcDef(std::shared_ptr<VNodeBase> node) {
}

void Visitor::mainFuncDef(std::shared_ptr<VNodeBase> node) {
}

void Visitor::decl(std::shared_ptr<VNodeBase> node) {
}