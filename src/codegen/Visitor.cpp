#include <codegen/Visitor.h>

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
        node->nextChild();
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

IntType::InternalType Visitor::constExp(std::shared_ptr<VNodeBase> node) {
    return calConstExp(*node->getChildIter());
}
IntType::InternalType Visitor::calConstExp(std::shared_ptr<VNodeBase> node) {
    if (node->getType() == VType::VT) {
        if (expect(node, SymbolEnum::INTCON)) {
            return std::dynamic_pointer_cast<VNodeLeaf>(node)->getToken().value;
        }
        return std::dynamic_pointer_cast<VNodeLeaf>(node)->getToken().value;
    } else {
        switch (node->getNodeEnum()) {
        case VNodeEnum::ADDEXP:
            if (node->getChildrenNum() == 1) {
                return calConstExp(*node->getChildIter());
            } else {
                if ((*node->getChildIter(1))->getSymbol() == SymbolEnum::PLUS) {
                    return calConstExp(*node->getChildIter()) + calConstExp(*node->getChildIter(2));
                } else {
                    return calConstExp(*node->getChildIter()) - calConstExp(*node->getChildIter(2));
                }
            }
            break;
        case VNodeEnum::MULEXP:
            if (node->getChildrenNum() == 1) {
                return calConstExp(*node->getChildIter());
            } else {
                if ((*node->getChildIter(1))->getSymbol() == SymbolEnum::MULT) {
                    return calConstExp(*node->getChildIter()) * calConstExp(*node->getChildIter(2));
                } else if ((*node->getChildIter(1))->getSymbol() == SymbolEnum::DIV) {
                    return calConstExp(*node->getChildIter()) / calConstExp(*node->getChildIter(2));
                } else {
                    return calConstExp(*node->getChildIter()) % calConstExp(*node->getChildIter(2));
                }
            }
            break;
        case VNodeEnum::UNARYEXP:
            if (expect(*node->getChildIter(), SymbolEnum::IDENFR)) { // func
                PARSER_LOG_ERROR("Function can't not be parsed as constexpr!");
            } else if (expect(*node->getChildIter(), VNodeEnum::PRIMARYEXP)) {
                return calConstExp(*node->getChildIter());
            } else {
                auto symbol = (*(*node->getChildIter())->getChildIter())->getSymbol();
                if (symbol == SymbolEnum::PLUS) {
                    return calConstExp(*node->getChildIter(1));
                } else if (symbol == SymbolEnum::MINU) {
                    return -calConstExp(*node->getChildIter(1));
                } else {
                    PARSER_LOG_ERROR("Symbol: '!' should not be in constexpr!");
                }
            }
            break;
        case VNodeEnum::PRIMARYEXP:
            if (expect(*node->getChildIter(), SymbolEnum::LPARENT)) {
                calConstExp(*node->getChildIter(1));
            } else if (expect(*node->getChildIter(), VNodeEnum::LVAL)) {
                calConstExp(*node->getChildIter());
            } else {
                calConstExp(*node->getChildIter());
            }
            break;
        case VNodeEnum::LVAL: {
            auto leafNode = std::dynamic_pointer_cast<VNodeLeaf>(*node->getChildIter());
            auto item = m_table.findItem(leafNode->getToken().literal);
            auto constVarItem = dynamic_cast<ConstVarItem<IntType>*>(item);
            if (constVarItem) {
                return constVarItem->getConstVar();
            } else {
                std::vector<size_t> dims;
                while (expect(*node->getChildIter(), SymbolEnum::LBRACK) && expect(*node->getChildIter(2), SymbolEnum::RBRACK)) {
                    dims.push_back(std::dynamic_pointer_cast<VNodeLeaf>(*node->getChildIter(1))->getToken().value);
                    node->nextChild(3); // jump '[dim]'
                }
                auto constArrayItem = dynamic_cast<ConstVarItem<ArrayType<IntType>>*>(item);
                return constArrayItem->getConstVar()[constArrayItem->getType().getValueIndex(std::move(dims))];
            }
        } break;
        case VNodeEnum::EXP:
        case VNodeEnum::NUM:
            return calConstExp(*node->getChildIter());
            break;
        default: break;
        }
    }
    DBG_LOG("Can not calculate constexpr value!");
    return 0;
}

template <>
typename IntType::InternalType Visitor::constInitVal<IntType>(std::shared_ptr<VNodeBase> node) {
    return constExp(*node->getChildIter());
};

template <>
typename ArrayType<IntType>::InternalType Visitor::constInitVal<ArrayType<IntType>>(std::shared_ptr<VNodeBase> node) {
    typename ArrayType<IntType>::InternalType values;
    if (expect(*node->getChildIter(), SymbolEnum::LBRACE)) {
        node->nextChild();
        if (!expect(*node->getChildIter(), SymbolEnum::RBRACE)) {
            auto value = constInitVal<ArrayType<IntType>>(*node->getChildIter());
            values.insert(values.end(), value.begin(), value.end());
            node->nextChild(); // jump '}'
            while (expect(*node->getChildIter(), SymbolEnum::COMMA)) {
                node->nextChild(); // jump ','
                auto value = constInitVal<ArrayType<IntType>>(*node->getChildIter());
                values.insert(values.end(), value.begin(), value.end());
                node->nextChild(); // jump '}'
            }
        }
    }
    return values;
};

void Visitor::constDef(std::shared_ptr<VNodeBase> node) {
    auto leafNode = std::dynamic_pointer_cast<VNodeLeaf>(node);
    std::string identName = leafNode->getToken().literal;
    node->nextChild(); // jump IDENT
    std::vector<size_t> dims;
    while (expect(*node->getChildIter(), SymbolEnum::LBRACK) && expect(*node->getChildIter(2), SymbolEnum::RBRACK)) {
        dims.push_back(std::dynamic_pointer_cast<VNodeLeaf>(*node->getChildIter(1))->getToken().value);
        node->nextChild(3); // jump '[dim]'
    }
    node->nextChild(); // jump '='
    const size_t dimSize = dims.size();
    if (dimSize == 0) {
        auto value = constInitVal<IntType>(*node->getChildIter());
        m_table.insertItem<ConstVarItem<IntType>>(identName,
                                                  {.parentHandle = m_table.getCurrentScopeHandle(),
                                                   .constVar = value});
    } else {
        auto value = constInitVal<ArrayType<IntType>>(*node->getChildIter());
        m_table.insertItem<ConstVarItem<ArrayType<IntType>>>(identName, {.parentHandle = m_table.getCurrentScopeHandle(),
                                                                         .constVar = value});
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