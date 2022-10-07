#include <codegen/Visitor.h>

#ifndef DEBUG
#define DBG_PROBE_BRANCH(name, node) auto name = (node)->getNodeEnum()
#define DBG_PROBE_LEAF(name, node) auto name = (node)->getSymbol()
#define DBG_PROBE_VAL(val, expr) auto val = expr
#endif

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
        if (!node->nextChild()) break;
    }
    while (expect(*node->getChildIter(), VNodeEnum::FUNCDEF)) {
        funcDef(*node->getChildIter());
        if (!node->nextChild()) break;
    }
    if (expect(*node->getChildIter(), VNodeEnum::MAINFUNCDEF)) {
        mainFuncDef(*node->getChildIter());
    }

    m_table.popScope();
}

void Visitor::decl(std::shared_ptr<VNodeBase> node) {
    DBG_PROBE_BRANCH(type, *node->getChildIter());
    if (expect(*node->getChildIter(), VNodeEnum::CONSTDECL)) {
        constDecl(*node->getChildIter());
    } else {
        varDecl(*node->getChildIter());
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
            if (!node->nextChild()) break; // jump <ConstDef>
        }
    }
}

void Visitor::varDecl(std::shared_ptr<VNodeBase> node) {
    if (bType(*node->getChildIter()) == ValueTypeEnum::INT_TYPE) {
        node->nextChild(); // jump 'int'

        varDef(*node->getChildIter());
        node->nextChild(); // jump <VarDef>
        while (expect(*node->getChildIter(), SymbolEnum::COMMA)) {
            node->nextChild(); // jump ','
            varDef(*node->getChildIter());
            if (!node->nextChild()) break; // jump <VarDef>
        }
    }
}

IntType::InternalType Visitor::constExp(std::shared_ptr<VNodeBase> node) {
    return calConstExp(*node->getChildIter());
}

VarItem<IntType>* Visitor::exp(std::shared_ptr<VNodeBase> node) {
    // TODO: 实现exp递归下降
    return nullptr;
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
                Logger::logError("Function can't not be parsed as constexpr!");
            } else if (expect(*node->getChildIter(), VNodeEnum::PRIMARYEXP)) {
                return calConstExp(*node->getChildIter());
            } else {
                auto symbol = (*(*node->getChildIter())->getChildIter())->getSymbol();
                if (symbol == SymbolEnum::PLUS) {
                    return calConstExp(*node->getChildIter(1));
                } else if (symbol == SymbolEnum::MINU) {
                    return -calConstExp(*node->getChildIter(1));
                } else {
                    Logger::logError("Symbol: '!' should not be in constexpr!");
                }
            }
            break;
        case VNodeEnum::PRIMARYEXP:
            if (expect(*node->getChildIter(), SymbolEnum::LPARENT)) {
                return calConstExp(*node->getChildIter(1));
            } else if (expect(*node->getChildIter(), VNodeEnum::LVAL)) {
                return calConstExp(*node->getChildIter());
            } else {
                return calConstExp(*node->getChildIter());
            }
            break;
        case VNodeEnum::LVAL: {
            auto leafNode = std::dynamic_pointer_cast<VNodeLeaf>(*node->getChildIter());
            auto item = m_table.findItem(leafNode->getToken().literal);
            if (item != nullptr) {
                auto constVarItem = dynamic_cast<ConstVarItem<IntType>*>(item);
                if (constVarItem) {
                    return constVarItem->getConstVar();
                } else {
                    std::vector<size_t> dims;
                    node->nextChild();
                    while (expect(*node->getChildIter(), SymbolEnum::LBRACK) && expect(*node->getChildIter(2), SymbolEnum::RBRACK)) {
                        int dim = static_cast<size_t>(constExp(*node->getChildIter(1)));
                        if (dim >= 0) {
                            dims.push_back(static_cast<size_t>(dim));
                        } else {
                            Logger::logError("Use dimension as negative size!");
                        }
                        if (!node->nextChild(3)) break; // jump '[dim]'
                    }
                    auto constArrayItem = dynamic_cast<ConstVarItem<ArrayType<IntType>>*>(item);
                    auto val = constArrayItem->getConstVar()[std::move(dims)];
                    return val;
                }
            } else {
                Logger::logError(ErrorType::UNDECL_IDENT, leafNode->getToken().lineNum, leafNode->getToken().literal);
            }

        } break;
        case VNodeEnum::EXP:
        case VNodeEnum::NUM:
            return calConstExp(*node->getChildIter());
            break;
        default: break;
        }
    }
    DBG_ERROR("Can not calculate constexpr value!");
    return 0;
}

template <>
typename IntType::InternalType Visitor::constInitVal<IntType>(std::shared_ptr<VNodeBase> node, std::vector<size_t>& dims, int level) {
    return constExp(*node->getChildIter());
};

template <>
typename ArrayType<IntType>::InternalType Visitor::constInitVal<ArrayType<IntType>>(std::shared_ptr<VNodeBase> node, std::vector<size_t>& dims, int level) {
    typename ArrayType<IntType>::InternalType values;
    if (expect(*node->getChildIter(), SymbolEnum::LBRACE)) {
        node->nextChild();
        size_t num = 0;
        if (!expect(*node->getChildIter(), SymbolEnum::RBRACE)) {
            auto value = constInitVal<ArrayType<IntType>>(*node->getChildIter(), dims, level + 1);
            values.append(std::move(value));
            node->nextChild(); // jump '}'
            num++;
            while (expect(*node->getChildIter(), SymbolEnum::COMMA)) {
                node->nextChild(); // jump ','
                auto value = constInitVal<ArrayType<IntType>>(*node->getChildIter(), dims, level + 1);
                values.append(std::move(value));
                node->nextChild(); // jump '}'
                num++;
            }
        }
        //std::cout << num << " " << level << std::endl;
        int diff = dims[level] - num; // 补零和报错
        if (diff >= 0) {
            for (int i = 0; i < diff; i++) {
                if (level + 1 >= dims.size()) {
                    values.insert(0);
                } else {
                    for (int j = 0; j < dims[level + 1]; j++) {
                        values.insert(0);
                    }
                }
            }
        } else {
            Logger::logError("Too much number defined!");
        }
    } else {
        IntType::InternalType val = constInitVal<IntType>(*node->getChildIter(), dims, level + 1);
        values.insert(val);
    }
    values.setDimensions(dims);
    return values;
};

template <>
IntType::InternalItem Visitor::initVal<IntType>(std::shared_ptr<VNodeBase> node, std::vector<size_t>& dims, int level) {
    return exp(*node->getChildIter());
};

template <>
typename ArrayType<IntType>::InternalItem Visitor::initVal<ArrayType<IntType>>(std::shared_ptr<VNodeBase> node, std::vector<size_t>& dims, int level) {
    typename ArrayType<IntType>::InternalItem values;
    if (expect(*node->getChildIter(), SymbolEnum::LBRACE)) {
        node->nextChild();
        size_t num = 0;
        if (!expect(*node->getChildIter(), SymbolEnum::RBRACE)) {
            auto value = initVal<ArrayType<IntType>>(*node->getChildIter(), dims, level + 1);
            values.append(std::move(value));
            node->nextChild(); // jump '}'
            num++;
            while (expect(*node->getChildIter(), SymbolEnum::COMMA)) {
                node->nextChild(); // jump ','
                auto value = initVal<ArrayType<IntType>>(*node->getChildIter(), dims, level + 1);
                values.append(std::move(value));
                node->nextChild(); // jump '}'
                num++;
            }
        }
    } else {
        IntType::InternalItem val = initVal<IntType>(*node->getChildIter(), dims, level + 1);
        values.insert(val);
    }
    return values;
};

template <>
typename IntType::InternalType Visitor::initValGlobal<IntType>(std::shared_ptr<VNodeBase> node, std::vector<size_t>& dims, int level) {
    return constInitVal<IntType>(node, dims, level);
};

template <>
typename ArrayType<IntType>::InternalType Visitor::initValGlobal<ArrayType<IntType>>(std::shared_ptr<VNodeBase> node, std::vector<size_t>& dims, int level) {
    return constInitVal<ArrayType<IntType>>(node, dims, level);
};

void Visitor::constDef(std::shared_ptr<VNodeBase> node) {
    auto leafNode = std::dynamic_pointer_cast<VNodeLeaf>(*node->getChildIter());
    std::string identName = leafNode->getToken().literal;
    int lineNum = leafNode->getToken().lineNum;
    node->nextChild(); // jump IDENT
    std::vector<size_t> dims;
    while (expect(*node->getChildIter(), SymbolEnum::LBRACK) && expect(*node->getChildIter(2), SymbolEnum::RBRACK)) {
        int dim = constExp(*node->getChildIter(1));
        if (dim >= 0) {
            dims.push_back(static_cast<size_t>(dim));
        } else {
            Logger::logError("Use dimension as negative size!");
        }
        node->nextChild(3); // jump '[dim]'
    }
    node->nextChild(); // jump '='
    std::pair<SymbolTableItem*, bool> res;
    if (dims.size() == 0) {
        auto value = constInitVal<IntType>(*node->getChildIter(), dims, 0);
        res = m_table.insertItem<ConstVarItem<IntType>>(identName,
                                                        {.parentHandle = m_table.getCurrentScopeHandle(),
                                                         .constVar = value});

    } else {
        auto value = constInitVal<ArrayType<IntType>>(*node->getChildIter(), dims, 0);
        res = m_table.insertItem<ConstVarItem<ArrayType<IntType>>>(identName, {.parentHandle = m_table.getCurrentScopeHandle(),
                                                                               .constVar = value});
    }
    if (!res.second) {
        Logger::logError(ErrorType::REDEF_IDENT, lineNum, identName);
    }
}

void Visitor::varDef(std::shared_ptr<VNodeBase> node) {
    auto leafNode = std::dynamic_pointer_cast<VNodeLeaf>(*node->getChildIter());
    std::string identName = leafNode->getToken().literal;
    int lineNum = leafNode->getToken().lineNum;
    node->nextChild(); // jump IDENT
    std::vector<size_t> dims;
    while (expect(*node->getChildIter(), SymbolEnum::LBRACK) && expect(*node->getChildIter(2), SymbolEnum::RBRACK)) {
        int dim = constExp(*node->getChildIter(1));
        if (dim >= 0) {
            dims.push_back(static_cast<size_t>(dim));
        } else {
            Logger::logError("Use dimension as negative size!");
        }
        node->nextChild(3); // jump '[dim]'
    }
    std::pair<SymbolTableItem*, bool> res(nullptr, true);
    if (m_table.getCurrentScope().getType() == BlockScopeType::GLOBAL) {
        MultiFlatArray<int> valueArray;
        int valueVar = 0;
        if (expect(*node->getChildIter(), SymbolEnum::ASSIGN)) {
            node->nextChild();
            if (dims.size() == 0) {
                auto value = initValGlobal<IntType>(*node->getChildIter(), dims, 0);
            } else {
                auto value = initValGlobal<ArrayType<IntType>>(*node->getChildIter(), dims, 0);
            }
        }
        if (dims.size() == 0) {
            res = m_table.insertItem<VarItem<IntType>>(identName,
                                                       {.parentHandle = m_table.getCurrentScopeHandle(),
                                                        .initVar = valueVar});
        } else {
            res = m_table.insertItem<VarItem<ArrayType<IntType>>>(identName,
                                                                  {.parentHandle = m_table.getCurrentScopeHandle(),
                                                                   .initVar = valueArray});
        }

    } else {
        // TODO: 局部变量的初始化处理
    }
    if (!res.second) {
        Logger::logError(ErrorType::REDEF_IDENT, lineNum, identName);
    }
}

ValueTypeEnum Visitor::bType(std::shared_ptr<VNodeBase> node) { // 基本类型
    return ValueTypeEnum::INT_TYPE;
}

ValueTypeEnum Visitor::funcType(std::shared_ptr<VNodeBase> node) {
    if ((*node->getChildIter())->getSymbol() == SymbolEnum::INTTK) {
        return ValueTypeEnum::INT_TYPE;
    } else {
        return ValueTypeEnum::VOID_TYPE;
    }
}
void Visitor::mainFuncDef(std::shared_ptr<VNodeBase> node) {
    node->nextChild(); // jump 'int' | 'void'
    auto leafNode = std::dynamic_pointer_cast<VNodeLeaf>(*node->getChildIter());
    std::string identName = "main";
    int lineNum = leafNode->getToken().lineNum;
    auto res = m_table.insertItem<FuncItem>(identName, {.parentHandle = m_table.getCurrentScopeHandle(), .retType = ValueTypeEnum::INT_TYPE});
    if (!res.second) {
        Logger::logError(ErrorType::REDEF_IDENT, lineNum, identName);
    }
    node->nextChild(2); // jump MAINTK & '('
    m_table.pushScope(BlockScopeType::FUNC);
    m_table.getCurrentScope().setFuncItem(res.first);
    std::vector<SymbolTableItem*> params;
    if (expect(*node->getChildIter(), VNodeEnum::FUNCFPARAMS)) {
        params = funcFParams(*node->getChildIter());
        node->nextChild();
    }
    res.first->setParams(std::move(params));
    node->nextChild(); // jump ')'
    block(*node->getChildIter());
    m_table.popScope();
}

void Visitor::funcDef(std::shared_ptr<VNodeBase> node) {
    auto retType = funcType(*node->getChildIter());
    node->nextChild(); // jump 'int' | 'void'
    auto leafNode = std::dynamic_pointer_cast<VNodeLeaf>(*node->getChildIter());
    std::string identName = leafNode->getToken().literal;
    int lineNum = leafNode->getToken().lineNum;
    auto res = m_table.insertItem<FuncItem>(identName, {.parentHandle = m_table.getCurrentScopeHandle(), .retType = retType});
    if (!res.second) {
        Logger::logError(ErrorType::REDEF_IDENT, lineNum, identName);
    }
    node->nextChild(2); // jump IDENT '('
    m_table.pushScope(BlockScopeType::FUNC);
    m_table.getCurrentScope().setFuncItem(res.first);
    std::vector<SymbolTableItem*> params;
    if (expect(*node->getChildIter(), VNodeEnum::FUNCFPARAMS)) {
        params = funcFParams(*node->getChildIter());
        node->nextChild();
    }
    res.first->setParams(std::move(params));
    node->nextChild(); // jump ')'
    block(*node->getChildIter());
    m_table.popScope(); // pop from func
}

// TODO: 完成形参列表
std::vector<SymbolTableItem*> Visitor::funcFParams(std::shared_ptr<VNodeBase> node) {
    std::vector<SymbolTableItem*> params;
    params.push_back(funcFParam(*node->getChildIter()));
    node->nextChild();
    while (expect(*node->getChildIter(), SymbolEnum::COMMA)) {
        node->nextChild(); // jump ','
        params.push_back(funcFParam(*node->getChildIter()));
    }
    return params;
}
SymbolTableItem* Visitor::funcFParam(std::shared_ptr<VNodeBase> node) {
    if (bType(*node->getChildIter()) == ValueTypeEnum::INT_TYPE) {
        node->nextChild(); // jump 'int'
        auto leafNode = std::dynamic_pointer_cast<VNodeLeaf>(*node->getChildIter());
        std::string identName = leafNode->getToken().literal;
        int lineNum = leafNode->getToken().lineNum;
        std::vector<size_t> dims;
        if (node->nextChild()) {
            if (expect(*node->getChildIter(), SymbolEnum::LBRACK) && expect(*node->getChildIter(1), SymbolEnum::RBRACK)) {
                dims.push_back(0);
                if (node->nextChild(2)) {
                    while (expect(*node->getChildIter(), SymbolEnum::LBRACK) && expect(*node->getChildIter(2), SymbolEnum::RBRACK)) {
                        int dim = static_cast<size_t>(constExp(*node->getChildIter(1)));
                        if (dim >= 0) {
                            dims.push_back(static_cast<size_t>(dim));
                        } else {
                            Logger::logError("Use dimension as negative size!");
                        }
                        if (!node->nextChild(3)) break; // jump '[dim]'
                    }
                }
            }
        }

        SymbolTableItem* ret = nullptr;
        bool valid = true;
        if (dims.size() == 0) {
            auto res = m_table.insertItem<VarItem<IntType>>(identName, {.parentHandle = m_table.getCurrentScopeHandle(), .initVar = 0, .varItem = nullptr});
            ret = res.first;
            valid = res.second;
        } else {
            auto res = m_table.insertItem<VarItem<ArrayType<IntType>>>(identName, {.parentHandle = m_table.getCurrentScopeHandle(),
                                                                                   .initVar = {{.values = {}, .dimensions = dims}},
                                                                                   .varItem = {{.values = {}, .dimensions = dims}}});
            ret = res.first;
            valid = res.second;
        }
        if (!valid) {
            Logger::logError(ErrorType::REDEF_IDENT, lineNum, identName);
        }
        ret->setParam();
        return ret;
    }
    return nullptr;
}

void Visitor::block(std::shared_ptr<VNodeBase> node) {
    node->nextChild(); // jump '{'
    auto& children = node->getChildren();
    for (auto it = node->getChildIter(); it != children.end() - 1; it++) {
        blockItem(*it);
    }
    node->nextChild(); // jump '}'
}

void Visitor::blockItem(std::shared_ptr<VNodeBase> node) {
    if (expect(*node->getChildIter(), VNodeEnum::DECL)) {
        decl(*node->getChildIter());
    } else {
        stmt(*node->getChildIter());
    }
}

void Visitor::stmt(std::shared_ptr<VNodeBase> node) {
    if (expect(*node->getChildIter(), SymbolEnum::IFTK)) { // if
        node->nextChild(2);                                // jump  IFTK & '('
        cond(*node->getChildIter());
        node->nextChild(); // jump ')'
        m_table.pushScope(BlockScopeType::BRANCH);
        stmt(*node->getChildIter());
        m_table.popScope();
        node->nextChild(); // jump STMT
        if (expect(*node->getChildIter(), SymbolEnum::ELSETK)) {
            node->nextChild(); // jump ELSETK
            m_table.pushScope(BlockScopeType::BRANCH);
            stmt(*node->getChildIter());
            m_table.popScope();
        }
    } else if (expect(*node->getChildIter(), SymbolEnum::WHILETK)) {
        node->nextChild(2); // jump WHILE & '('
        cond(*node->getChildIter());
        node->nextChild();
        m_table.pushScope(BlockScopeType::LOOP);
        stmt(*node->getChildIter());
        m_table.popScope();
        node->nextChild(); // jump STMT
    } else if (expect(*node->getChildIter(), SymbolEnum::BREAKTK)) {
        node->nextChild(); // jump BREAKTK
    } else if (expect(*node->getChildIter(), SymbolEnum::CONTINUETK)) {
        node->nextChild(); // jump CONTINUETK
    } else if (expect(*node->getChildIter(), SymbolEnum::RETURNTK)) {
        auto leafNode = std::dynamic_pointer_cast<VNodeLeaf>(*node->getChildIter());
        node->nextChild(); // jump RETURNTK
        FuncItem* funcItem = m_table.getCurrentScope().getFuncItem();
        ValueTypeEnum type = funcItem->getReturnValueType();
        std::string funcName = funcItem->getName();
        int lineNum = leafNode->getToken().lineNum;
        if (expect(*node->getChildIter(), VNodeEnum::EXP)) {
            if (type == ValueTypeEnum::VOID_TYPE) {
                Logger::logError(ErrorType::VOID_FUNC_HAVE_RETURNED, lineNum, funcName);
            }
            exp(*node->getChildIter());
            node->nextChild(); // jump EXP
        } else {
            if (type == ValueTypeEnum::INT_TYPE) {
                Logger::logError(ErrorType::NONVOID_FUNC_MISS_RETURN, lineNum, funcName); // 这里表示有return 但是没有返回值的情况
                // TODO: 是INT_TYPE 但是没有出现return关键字的情况
            }
        }
    } else if (expect(*node->getChildIter(), SymbolEnum::PRINTFTK)) {
        node->nextChild(2); // jump PRINTTK & '('
        auto leafNode = std::dynamic_pointer_cast<VNodeLeaf>(*node->getChildIter());
        node->nextChild(); // jump STRCON
        const std::string& formatStr = leafNode->getToken().literal;
        int lineNum = leafNode->getToken().lineNum;
        int count = 0;
        std::string sub = "%d";
        for (size_t offset = formatStr.find(sub); offset != std::string::npos;
             offset = formatStr.find(sub, offset + 2)) {
            count++;
        }
        std::vector<VarItem<IntType>*> items;
        while (expect(*node->getChildIter(), SymbolEnum::COMMA)) {
            node->nextChild(); // jump ','
            items.push_back(exp(*node->getChildIter()));
            node->nextChild(); // jump EXP
        }
        if (items.size() != count) {
            Logger::logError(ErrorType::PRINTF_UMATCHED, lineNum, std::to_string(items.size()), std::to_string(count));
        }
    } else if (expect(*node->getChildIter(), VNodeEnum::LVAL)) {
        // TODO: lVal int stmt
        // auto leafNode = std::dynamic_pointer_cast<VNodeLeaf>(*node->getChildIter());
        // std::string identName = leafNode->getToken().literal;
        // auto finded = m_table.findItem(identName);
        // if (finded) {
        // }
    } else if (expect(*node->getChildIter(), VNodeEnum::BLOCK)) {
    } else if (expect(*node->getChildIter(), VNodeEnum::EXP)) {
    }
}

void Visitor::cond(std::shared_ptr<VNodeBase> node) {
}
