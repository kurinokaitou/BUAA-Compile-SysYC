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
    while (expect(*node->getChildIter(), VNodeEnum::DECL)) {
        decl(*node->getChildIter());
        if (node->nextChild()) break;
    }
    while (expect(*node->getChildIter(), VNodeEnum::FUNCDEF)) {
        funcDef(*node->getChildIter());
        if (node->nextChild()) break;
    }
    if (expect(*node->getChildIter(), VNodeEnum::MAINFUNCDEF)) {
        mainFuncDef(*node->getChildIter());
    }

    m_table.popScope();
}

void Visitor::decl(std::shared_ptr<VNodeBase> node) {
    if (expect(*node->getChildIter(), SymbolEnum::CONSTTK)) {
        varDecl(*node->getChildIter());
    } else {
        constDecl(*node->getChildIter());
    }
}

void Visitor::constDecl(std::shared_ptr<VNodeBase> node) {
    node->nextChild(); // jump 'const'
    if (bType(*node->getChildIter()) == ValueTypeEnum::INT_TYPE) {
        node->nextChild(); // jump 'int'
        constDef(*node->getChildIter());
        node->nextChild(); // jump <ConstDef>
        while (expect(*node->getChildIter(), SymbolEnum::COMMA)) {
            node->nextChild(); // jump ','
            constDef(*node->getChildIter());
            if (node->nextChild()) break; // jump <ConstDef>
        }
    }
}

void Visitor::varDecl(std::shared_ptr<VNodeBase> node) {
}

void Visitor::constDef(std::shared_ptr<VNodeBase> node) {
    auto leafNode = std::dynamic_pointer_cast<VNodeLeaf>(node);
    std::string identName = leafNode->getToken().literal;
    node->nextChild(); // jump IDENT
    std::vector<int> dimensions;
    while (expect(*node->getChildIter(), SymbolEnum::LBRACK) && expect(*node->getChildIter(2), SymbolEnum::RBRACE)) {
        dimensions.push_back(std::dynamic_pointer_cast<VNodeLeaf>(*node->getChildIter(1))->getToken().value);
        node->nextChild(3); // jump '[dimension]'
    }
    node->nextChild(); // jump '='
    if (dimensions.size() == 0) {
        auto value = constInitVal<IntType>(*node->getChildIter());
        m_table.insertItem<ConstVarItem<IntType>>(identName,
                                                  {.parentHandle = m_table.getCurrentScopeHandle(),
                                                   .constVar = value});
    } else {
        // TODO: handle multi dimension array type
    }
}

ValueTypeEnum Visitor::bType(std::shared_ptr<VNodeBase> node) { // 基本类型
    if (node->getSymbol() == SymbolEnum::INTTK) {
        return ValueTypeEnum::INT_TYPE;
    } else {
        return ValueTypeEnum::VOID_TYPE;
    }
}

void Visitor::funcDef(std::shared_ptr<VNodeBase> node) {
}

void Visitor::mainFuncDef(std::shared_ptr<VNodeBase> node) {
}