#include <codegen/Visitor.h>

#ifndef NDEBUG
#define DBG_PROBE_BRANCH(name) auto name = (*node->getChildIter())->getNodeEnum()
#define DBG_PROBE_LEAF(name) auto name = (*node->getChildIter())->getSymbol()
#define DBG_PROBE_VAL(val, expr) auto val = expr
#else
#define DBG_PROBE_BRANCH(name, node)
#define DBG_PROBE_LEAF(name, node)
#define DBG_PROBE_VAL(val, expr)
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
        constDef<IntType>(*node->getChildIter());
        node->nextChild(); // jump <ConstDef>
        while (expect(*node->getChildIter(), SymbolEnum::COMMA)) {
            node->nextChild(); // jump ','
            constDef<IntType>(*node->getChildIter());
            if (!node->nextChild()) break; // jump <ConstDef>
        }
    } else if (bType(*node->getChildIter()) == ValueTypeEnum::CHAR_TYPE) {
        node->nextChild(); // jump 'int'
        constDef<CharType>(*node->getChildIter());
        node->nextChild(); // jump <ConstDef>
        while (expect(*node->getChildIter(), SymbolEnum::COMMA)) {
            node->nextChild(); // jump ','
            constDef<CharType>(*node->getChildIter());
            if (!node->nextChild()) break; // jump <ConstDef>
        }
    }
}

void Visitor::varDecl(std::shared_ptr<VNodeBase> node) {
    if (bType(*node->getChildIter()) == ValueTypeEnum::INT_TYPE) {
        node->nextChild(); // jump 'int'
        varDef<IntType>(*node->getChildIter());
        node->nextChild(); // jump <VarDef>
        while (expect(*node->getChildIter(), SymbolEnum::COMMA)) {
            node->nextChild(); // jump ','
            varDef<IntType>(*node->getChildIter());
            if (!node->nextChild()) break; // jump <VarDef>
        }
    } else if (bType(*node->getChildIter()) == ValueTypeEnum::CHAR_TYPE) {
        node->nextChild(); // jump 'int'
        varDef<CharType>(*node->getChildIter());
        node->nextChild(); // jump <VarDef>
        while (expect(*node->getChildIter(), SymbolEnum::COMMA)) {
            node->nextChild(); // jump ','
            varDef<CharType>(*node->getChildIter());
            if (!node->nextChild()) break; // jump <VarDef>
        }
    }
}

template <typename Type>
typename Type::InternalType Visitor::constExp(std::shared_ptr<VNodeBase> node) {
    DBG_PROBE_BRANCH(name);
    return calConstExp<Type>(*node->getChildIter());
}
template <typename Type>
SymbolTableItem* Visitor::exp(std::shared_ptr<VNodeBase> node) {
    DBG_PROBE_BRANCH(name);
    return addExp<Type>(*node->getChildIter());
}

template <typename Type>
SymbolTableItem* Visitor::addExp(std::shared_ptr<VNodeBase> node) {
    if (expect(*node->getChildIter(), VNodeEnum::MULEXP)) {
        return mulExp<Type>(*node->getChildIter());
    } else {
        auto add = addExp<Type>(*node->getChildIter());
        node->nextChild();
        SymbolEnum op = (*node->getChildIter())->getSymbol(); // get symbol of plus or minus
        node->nextChild();
        auto mul = mulExp<Type>(*node->getChildIter());
        auto ret = MAKE_VAR();
        if (op == SymbolEnum::PLUS) {
            // 生成加法的代码
        } else if (op == SymbolEnum::MINU) {
            // 生成减法的代码
        } else {
            DBG_ERROR("Add expression only accept '+' & '-'!");
            return nullptr;
        }
        return ret;
    }
}

template <typename Type>
SymbolTableItem* Visitor::mulExp(std::shared_ptr<VNodeBase> node) {
    if (expect(*node->getChildIter(), VNodeEnum::UNARYEXP)) {
        return unaryExp<Type>(*node->getChildIter());
    } else {
        auto mul = mulExp<Type>(*node->getChildIter());
        node->nextChild();
        SymbolEnum op = (*node->getChildIter())->getSymbol(); // get symbol of plus or minus
        node->nextChild();
        auto unary = unaryExp<Type>(*node->getChildIter());
        auto ret = MAKE_VAR();
        if (op == SymbolEnum::MULT) {
            // 生成乘法的代码
        } else if (op == SymbolEnum::DIV) {
            // 生成除法的代码
        } else if (op == SymbolEnum::MOD) {
            // 生成取余的代码
        } else {
            DBG_ERROR("Mul expression only accept '*' & '/' & '%'!");
            return nullptr;
        }
        return ret;
    }
}

template <typename Type>
SymbolTableItem* Visitor::unaryExp(std::shared_ptr<VNodeBase> node) {
    if (expect(*node->getChildIter(), VNodeEnum::PRIMARYEXP)) {
        return primaryExp<Type>(*node->getChildIter());
    } else if (expect(*node->getChildIter(), SymbolEnum::IDENFR)) {
        auto leafNode = std::dynamic_pointer_cast<VNodeLeaf>(*node->getChildIter());
        std::string identName = leafNode->getToken().literal;
        int lineNum = leafNode->getToken().lineNum;
        node->nextChild(2); // jump IDENT & '('
        std::vector<SymbolTableItem*> realParams;
        FuncItem* func = m_table.findFunc(identName);
        if (!func) {
            Logger::logError(ErrorType::UNDECL_IDENT, lineNum, identName);
        }
        if (expect(*node->getChildIter(), VNodeEnum::FUNCRPARAMS)) {
            realParams = funcRParams(*node->getChildIter(), func, lineNum); // 传入func
        }
        SymbolTableItem* ret = nullptr;
        if (func->getReturnValueType() == ValueTypeEnum::INT_TYPE) {
            ret = MAKE_INT_VAR();
        } else if (func->getReturnValueType() == ValueTypeEnum::CHAR_TYPE) {
            ret = MAKE_CHAR_VAR();
        } else {
            ret = MAKE_VOID_VAR();
        }

        // 生成函数调用，复制参数的代码，ret为返回值
        return ret;
    } else if (expect(*node->getChildIter(), VNodeEnum::UNARYOP)) {
        auto op = unaryOp(*node->getChildIter());
        node->nextChild();
        auto ret = unaryExp<Type>(*node->getChildIter());
        // 生成处理'-'和'!'的代码
        if (op == SymbolEnum::MINU) {
        } else if (op == SymbolEnum::NOT) {
        }
        return ret;
    } else {
        DBG_ERROR("Unary exppression can not handle input!");
        return nullptr;
    }
}

SymbolEnum Visitor::unaryOp(std::shared_ptr<VNodeBase> node) {
    return (*node->getChildIter())->getSymbol();
}

std::vector<SymbolTableItem*> Visitor::funcRParams(std::shared_ptr<VNodeBase> node, FuncItem* func, int lineNum) {
    std::vector<SymbolTableItem*> realParams;
    std::vector<std::shared_ptr<VNodeBase>> exps;
    for (auto& child : node->getChildren()) {
        if (expect(child, VNodeEnum::EXP)) {
            exps.push_back(child);
        }
    }
    if (func) {
        auto& formalParams = func->getParams();
        if (formalParams.size() == exps.size()) {
            bool match = true;
            for (size_t i = 0; i < formalParams.size(); i++) {
                auto type = formalParams[i]->getType()->getValueTypeEnum();
                auto isArray = formalParams[i]->getType()->isArray();

                SymbolTableItem* ret = nullptr;
                if (type == ValueTypeEnum::INT_TYPE) {
                    ret = funcRParam<IntType>(exps[i], formalParams[i]);
                } else {
                    ret = funcRParam<CharType>(exps[i], formalParams[i]);
                }
                if (ret) {
                    realParams.push_back(ret);
                } else {
                    match = false;
                }
            }
            if (!match) {
                Logger::logError(ErrorType::PARAMS_TYPE_NOT_MATCH, lineNum);
            }
        } else {
            Logger::logError(ErrorType::PARAMS_NUM_NOT_MATCH, lineNum, std::to_string(exps.size()), std::to_string(formalParams.size()));
        }
    }

    return realParams;
}

template <typename Type>
SymbolTableItem* Visitor::funcRParam(std::shared_ptr<VNodeBase> node, SymbolTableItem* formalParam) {
    auto isArray = formalParam->getType()->isArray();
    SymbolTableItem* ret = nullptr;
    if (isArray) {
        auto realArr = exp<ArrayType<Type>>(node);
        auto formalDims = getArrayItemDimensions<Type>(formalParam);
        auto realDims = getArrayItemDimensions<Type>(realArr);
        if (checkConvertiable<ArrayType<Type>>(realArr)) {
            ret = (realDims.size() == formalDims.size()) ? realArr : nullptr;
        }
    } else {
        auto realVar = exp<Type>(node);
        if (checkConvertiable<Type>(realVar)) {
            ret = realVar;
        }
    }
    return ret;
}

template <typename Type>
SymbolTableItem* Visitor::primaryExp(std::shared_ptr<VNodeBase> node) {
    if (expect(*node->getChildIter(), SymbolEnum::LPARENT)) {
        node->nextChild();
        return exp<Type>(*node->getChildIter());
    } else if (expect(*node->getChildIter(), VNodeEnum::LVAL)) {
        // 检查rVal返回的值类型是否与指定Type相同，如果不同给出警告并转换，不可转换则报错;
        return rVal(*node->getChildIter());
    } else {
        return number<Type>(*node->getChildIter());
    }
}

template <typename Type>
bool Visitor::checkConvertiable(SymbolTableItem* item) {
    SymbolTableItem* ret = nullptr;
    auto varValue = dynamic_cast<VarItem<Type>*>(item); // 直接转换，只有类型完全相同才能转化
    auto constVarValue = dynamic_cast<ConstVarItem<Type>*>(item);
    return (varValue != nullptr) || (constVarValue != nullptr);
}

template <typename Type>
VarItem<Type>* Visitor::number(std::shared_ptr<VNodeBase> node) {
    return MAKE_VAR();
}

template <>
VarItem<IntType>* Visitor::number(std::shared_ptr<VNodeBase> node) {
    auto leafNode = std::dynamic_pointer_cast<VNodeLeaf>(*node->getChildIter());
    auto lineNum = leafNode->getToken().lineNum;
    auto value = static_cast<typename IntType::InternalType>(leafNode->getToken().value);
    auto number = m_table.makeItem<VarItem<IntType>>({.parentHandle = m_table.getCurrentScopeHandle(), .initVar = value, .varItem = nullptr});
    // 生成代码
    return number;
}

template <>
VarItem<CharType>* Visitor::number(std::shared_ptr<VNodeBase> node) {
    auto leafNode = std::dynamic_pointer_cast<VNodeLeaf>(*node->getChildIter());
    auto value = static_cast<typename CharType::InternalType>(leafNode->getToken().value);
    auto number = m_table.makeItem<VarItem<CharType>>({.parentHandle = m_table.getCurrentScopeHandle(), .initVar = value, .varItem = nullptr});
    // 生成代码
    return number;
}

template <typename Type>
typename Type::InternalType Visitor::calConstExp(std::shared_ptr<VNodeBase> node) {
    if (node->getType() == VType::VT) {
        if (expect(node, SymbolEnum::INTCON)) {
            return std::dynamic_pointer_cast<VNodeLeaf>(node)->getToken().value;
        }
        return std::dynamic_pointer_cast<VNodeLeaf>(node)->getToken().value;
    } else {
        switch (node->getNodeEnum()) {
        case VNodeEnum::ADDEXP:
            if (node->getChildrenNum() == 1) {
                return calConstExp<Type>(*node->getChildIter());
            } else {
                if ((*node->getChildIter(1))->getSymbol() == SymbolEnum::PLUS) {
                    return calConstExp<Type>(*node->getChildIter()) + calConstExp<Type>(*node->getChildIter(2));
                } else {
                    return calConstExp<Type>(*node->getChildIter()) - calConstExp<Type>(*node->getChildIter(2));
                }
            }
            break;
        case VNodeEnum::MULEXP:
            if (node->getChildrenNum() == 1) {
                return calConstExp<Type>(*node->getChildIter());
            } else {
                if ((*node->getChildIter(1))->getSymbol() == SymbolEnum::MULT) {
                    return calConstExp<Type>(*node->getChildIter()) * calConstExp<Type>(*node->getChildIter(2));
                } else if ((*node->getChildIter(1))->getSymbol() == SymbolEnum::DIV) {
                    return calConstExp<Type>(*node->getChildIter()) / calConstExp<Type>(*node->getChildIter(2));
                } else {
                    return calConstExp<Type>(*node->getChildIter()) % calConstExp<Type>(*node->getChildIter(2));
                }
            }
            break;
        case VNodeEnum::UNARYEXP:
            if (expect(*node->getChildIter(), SymbolEnum::IDENFR)) { // func
                Logger::logError("Function can't not be parsed as constexpr!");
            } else if (expect(*node->getChildIter(), VNodeEnum::PRIMARYEXP)) {
                return calConstExp<Type>(*node->getChildIter());
            } else {
                auto symbol = (*(*node->getChildIter())->getChildIter())->getSymbol();
                if (symbol == SymbolEnum::PLUS) {
                    return calConstExp<Type>(*node->getChildIter(1));
                } else if (symbol == SymbolEnum::MINU) {
                    return -calConstExp<Type>(*node->getChildIter(1));
                } else {
                    Logger::logError("Symbol: '!' should not be in constexpr!");
                }
            }
            break;
        case VNodeEnum::PRIMARYEXP:
            if (expect(*node->getChildIter(), SymbolEnum::LPARENT)) {
                return calConstExp<Type>(*node->getChildIter(1));
            } else if (expect(*node->getChildIter(), VNodeEnum::LVAL)) {
                return calConstExp<Type>(*node->getChildIter());
            } else {
                return calConstExp<Type>(*node->getChildIter());
            }
            break;
        case VNodeEnum::LVAL: {
            auto leafNode = std::dynamic_pointer_cast<VNodeLeaf>(*node->getChildIter());
            auto item = m_table.findItem(leafNode->getToken().literal);
            if (item != nullptr) {
                auto constVarItem = dynamic_cast<ConstVarItem<Type>*>(item);
                if (constVarItem) {
                    return constVarItem->getConstVar();
                } else {
                    std::vector<size_t> dims;
                    node->nextChild();
                    while (expect(*node->getChildIter(), SymbolEnum::LBRACK) && expect(*node->getChildIter(2), SymbolEnum::RBRACK)) {
                        int dim = constExp<Type>(*node->getChildIter(1));
                        if (dim >= 0) {
                            dims.push_back(static_cast<size_t>(dim));
                        } else {
                            Logger::logError("Use dimension as negative size!");
                        }
                        if (!node->nextChild(3)) break; // jump '[dim]'
                    }
                    auto constArrayItem = dynamic_cast<ConstVarItem<ArrayType<Type>>*>(item);
                    auto val = constArrayItem->getConstVar()[std::move(dims)];
                    return val;
                }
            } else {
                Logger::logError(ErrorType::UNDECL_IDENT, leafNode->getToken().lineNum, leafNode->getToken().literal);
            }

        } break;
        case VNodeEnum::EXP:
        case VNodeEnum::NUM:
            return calConstExp<Type>(*node->getChildIter());
            break;
        default: break;
        }
    }
    DBG_ERROR("Can not calculate constexpr value!");
    return 0;
}

template <typename Type>
typename Type::InternalType Visitor::constInitVal(std::shared_ptr<VNodeBase> node, std::vector<size_t>& dims, int level) {
    DBG_PROBE_BRANCH(name);
    return constExp<Type>(node);
};

// TODO: char 模板化
template <typename Type>
typename ArrayType<Type>::InternalType Visitor::constInitValArray(std::shared_ptr<VNodeBase> node, std::vector<size_t>& dims, int level) {
    typename ArrayType<Type>::InternalType values;
    if (expect(*node->getChildIter(), SymbolEnum::LBRACE)) {
        node->nextChild();
        size_t num = 0;
        if (!expect(*node->getChildIter(), SymbolEnum::RBRACE)) {
            auto value = constInitValArray<Type>(*node->getChildIter(), dims, level + 1);
            values.append(std::move(value));
            node->nextChild(); // jump '}'
            num++;
            while (expect(*node->getChildIter(), SymbolEnum::COMMA)) {
                node->nextChild(); // jump ','
                auto value = constInitValArray<Type>(*node->getChildIter(), dims, level + 1);
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
        DBG_PROBE_BRANCH(name);
        typename Type::InternalType val = constInitVal<Type>(*node->getChildIter(), dims, level + 1);
        values.insert(val);
    }
    values.setDimensions(dims);
    return values;
};

template <typename Type>
typename Type::InternalItem Visitor::initVal(std::shared_ptr<VNodeBase> node, std::vector<size_t>& dims, int level) {
    DBG_PROBE_BRANCH(name);
    return exp<Type>(node);
};

template <typename Type>
typename ArrayType<Type>::InternalItem Visitor::initValArray(std::shared_ptr<VNodeBase> node, std::vector<size_t>& dims, int level) {
    typename ArrayType<Type>::InternalItem values;
    if (expect(*node->getChildIter(), SymbolEnum::LBRACE)) {
        node->nextChild();
        size_t num = 0;
        if (!expect(*node->getChildIter(), SymbolEnum::RBRACE)) {
            auto value = initValArray<Type>(*node->getChildIter(), dims, level + 1);
            values.append(std::move(value));
            node->nextChild(); // jump '}'
            num++;
            while (expect(*node->getChildIter(), SymbolEnum::COMMA)) {
                node->nextChild(); // jump ','
                auto value = initValArray<Type>(*node->getChildIter(), dims, level + 1);
                values.append(std::move(value));
                node->nextChild(); // jump '}'
                num++;
            }
        }
    } else {
        DBG_PROBE_BRANCH(name);
        typename Type::InternalItem val = initVal<Type>(*node->getChildIter(), dims, level + 1);
        values.insert(val);
    }
    values.setDimensions(dims);
    return values;
};

template <typename Type>
typename Type::InternalType Visitor::initValGlobal(std::shared_ptr<VNodeBase> node, std::vector<size_t>& dims, int level) {
    return constInitVal<Type>(node, dims, level);
};

template <typename Type>
typename ArrayType<Type>::InternalType Visitor::initValGlobalArray(std::shared_ptr<VNodeBase> node, std::vector<size_t>& dims, int level) {
    return constInitValArray<Type>(node, dims, level);
};

template <typename Type>
void Visitor::constDef(std::shared_ptr<VNodeBase> node) {
    auto leafNode = std::dynamic_pointer_cast<VNodeLeaf>(*node->getChildIter());
    std::string identName = leafNode->getToken().literal;
    int lineNum = leafNode->getToken().lineNum;
    node->nextChild(); // jump IDENT
    std::vector<size_t> dims;
    while (expect(*node->getChildIter(), SymbolEnum::LBRACK) && expect(*node->getChildIter(2), SymbolEnum::RBRACK)) {
        int dim = constExp<Type>(*node->getChildIter(1));
        if (dim >= 0) {
            dims.push_back(static_cast<size_t>(dim));
        } else {
            Logger::logError("Use dimension as negative size!");
        }
        if (!node->nextChild(3)) break; // jump '[dim]'
    }
    node->nextChild(); // jump '='
    std::pair<SymbolTableItem*, bool> res;
    if (dims.size() == 0) {
        auto value = constInitVal<Type>(*node->getChildIter(), dims, 0);
        res = m_table.insertItem<ConstVarItem<Type>>(identName,
                                                     {.parentHandle = m_table.getCurrentScopeHandle(),
                                                      .constVar = value});

    } else {
        auto value = constInitValArray<Type>(*node->getChildIter(), dims, 0);
        res = m_table.insertItem<ConstVarItem<ArrayType<Type>>>(identName, {.parentHandle = m_table.getCurrentScopeHandle(),
                                                                            .constVar = value});
    }
    if (!res.second) {
        Logger::logError(ErrorType::REDEF_IDENT, lineNum, identName);
    }
}
template <typename Type>
void Visitor::varDef(std::shared_ptr<VNodeBase> node) {
    auto leafNode = std::dynamic_pointer_cast<VNodeLeaf>(*node->getChildIter());
    std::string identName = leafNode->getToken().literal;
    int lineNum = leafNode->getToken().lineNum;
    node->nextChild(1, false); // jump IDENT
    std::vector<size_t> dims;
    while (expect(*node->getChildIter(), SymbolEnum::LBRACK) && expect(*node->getChildIter(2), SymbolEnum::RBRACK)) {
        int dim = constExp<Type>(*node->getChildIter(1));
        if (dim >= 0) {
            dims.push_back(static_cast<size_t>(dim));
        } else {
            Logger::logError("Use dimension as negative size!");
        }
        if (!node->nextChild(3, false)) break; // jump '[dim]'
    }
    std::pair<SymbolTableItem*, bool> res(nullptr, true);
    if (m_table.getCurrentScope().getType() == BlockScopeType::GLOBAL) {
        MultiFlatArray<typename Type::InternalType> valueArray;
        typename Type::InternalType valueVar = 0;
        if (expect(*node->getChildIter(), SymbolEnum::ASSIGN)) {
            node->nextChild();
            if (dims.size() == 0) {
                valueVar = initValGlobal<Type>(*node->getChildIter(), dims, 0);
            } else {
                valueArray = initValGlobalArray<Type>(*node->getChildIter(), dims, 0);
            }
        }
        if (dims.size() == 0) {
            res = m_table.insertItem<VarItem<Type>>(identName,
                                                    {.parentHandle = m_table.getCurrentScopeHandle(),
                                                     .initVar = valueVar,
                                                     .varItem = nullptr});
        } else {
            res = m_table.insertItem<VarItem<ArrayType<Type>>>(identName,
                                                               {.parentHandle = m_table.getCurrentScopeHandle(),
                                                                .initVar = valueArray,
                                                                .varItem = {{.values = {}, .dimensions = dims}}});
        }

    } else {
        MultiFlatArray<SymbolTableItem*> itemArray;
        SymbolTableItem* item;
        if (expect(*node->getChildIter(), SymbolEnum::ASSIGN)) {
            node->nextChild();
            if (dims.size() == 0) {
                item = initVal<Type>(*node->getChildIter(), dims, 0);
            } else {
                itemArray = initValArray<Type>(*node->getChildIter(), dims, 0);
            }
        } else {
            itemArray.setDimensions(dims);
        }
        if (dims.size() == 0) {
            res = m_table.insertItem<VarItem<Type>>(identName,
                                                    {.parentHandle = m_table.getCurrentScopeHandle(),
                                                     .initVar = 0,
                                                     .varItem = item});
        } else {
            res = m_table.insertItem<VarItem<ArrayType<Type>>>(identName,
                                                               {.parentHandle = m_table.getCurrentScopeHandle(),
                                                                .initVar = {{.values = {}, .dimensions = dims}},
                                                                .varItem = itemArray});
        }
    }
    if (!res.second) {
        Logger::logError(ErrorType::REDEF_IDENT, lineNum, identName);
    }
}

ValueTypeEnum Visitor::bType(std::shared_ptr<VNodeBase> node) { // 基本类型
    if ((*node->getChildIter())->getSymbol() == SymbolEnum::INTTK) {
        return ValueTypeEnum::INT_TYPE;
    } else {
        return ValueTypeEnum::CHAR_TYPE;
    }
}

ValueTypeEnum Visitor::funcType(std::shared_ptr<VNodeBase> node) {
    if ((*node->getChildIter())->getSymbol() == SymbolEnum::INTTK) {
        return ValueTypeEnum::INT_TYPE;
    } else if ((*node->getChildIter())->getSymbol() == SymbolEnum::CHARTK) {
        return ValueTypeEnum::CHAR_TYPE;
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
    node->nextChild(1, false);
    while (expect(*node->getChildIter(), SymbolEnum::COMMA)) {
        node->nextChild(); // jump ','
        params.push_back(funcFParam(*node->getChildIter()));
    }
    return params;
}

SymbolTableItem* Visitor::funcFParam(std::shared_ptr<VNodeBase> node) {
    auto type = bType(*node->getChildIter());
    node->nextChild(); // jump 'int'
    auto leafNode = std::dynamic_pointer_cast<VNodeLeaf>(*node->getChildIter());
    std::string identName = leafNode->getToken().literal;
    int lineNum = leafNode->getToken().lineNum;
    std::vector<size_t> dims;
    if (node->nextChild(1, false)) {
        if (expect(*node->getChildIter(), SymbolEnum::LBRACK) && expect(*node->getChildIter(1), SymbolEnum::RBRACK)) {
            dims.push_back(0);
            if (node->nextChild(2, false)) {
                while (expect(*node->getChildIter(), SymbolEnum::LBRACK) && expect(*node->getChildIter(2), SymbolEnum::RBRACK)) {
                    int dim = constExp<IntType>(*node->getChildIter(1));
                    if (dim >= 0) {
                        dims.push_back(static_cast<size_t>(dim));
                    } else {
                        Logger::logError("Use dimension as negative size!");
                    }
                    if (!node->nextChild(3, false)) break; // jump '[dim]'
                }
            }
        }
    }
    SymbolTableItem* ret = nullptr;
    bool valid = true;
    if (type == ValueTypeEnum::INT_TYPE) {
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
    } else if (type == ValueTypeEnum::CHAR_TYPE) {
        if (dims.size() == 0) {
            auto res = m_table.insertItem<VarItem<CharType>>(identName, {.parentHandle = m_table.getCurrentScopeHandle(), .initVar = 0, .varItem = nullptr});
            ret = res.first;
            valid = res.second;
        } else {
            auto res = m_table.insertItem<VarItem<ArrayType<CharType>>>(identName, {.parentHandle = m_table.getCurrentScopeHandle(),
                                                                                    .initVar = {{.values = {}, .dimensions = dims}},
                                                                                    .varItem = {{.values = {}, .dimensions = dims}}});
            ret = res.first;
            valid = res.second;
        }
    }
    if (!valid) {
        Logger::logError(ErrorType::REDEF_IDENT, lineNum, identName);
    }
    ret->setParam();
    return ret;
    return nullptr;
}

void Visitor::block(std::shared_ptr<VNodeBase> node) {
    node->nextChild(); // jump '{'
    while (expect(*node->getChildIter(), VNodeEnum::BLOCKITEM)) {
        blockItem(*node->getChildIter());
        node->nextChild();
    }
    int lineNum = std::dynamic_pointer_cast<VNodeLeaf>(*node->getChildIter())->getToken().lineNum;
    if (m_table.getCurrentScope().getType() == BlockScopeType::FUNC) {
        if (m_table.getCurrentScope().getFuncItem()->getReturnValueType() != ValueTypeEnum::VOID_TYPE) {
            m_table.getCurrentScope().checkFuncScopeReturn(lineNum);
        }
    }
    // node->nextChild(); // jump '}'
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
        auto leafNode = std::dynamic_pointer_cast<VNodeLeaf>(*node->getChildIter());
        if (m_table.getCurrentScope().getType() != BlockScopeType::LOOP) {
            Logger::logError(ErrorType::BRK_CONT_NOT_IN_LOOP, leafNode->getToken().lineNum);
        }
        node->nextChild(); // jump BREAKTK
    } else if (expect(*node->getChildIter(), SymbolEnum::CONTINUETK)) {
        auto leafNode = std::dynamic_pointer_cast<VNodeLeaf>(*node->getChildIter());
        if (m_table.getCurrentScope().getType() != BlockScopeType::LOOP) {
            Logger::logError(ErrorType::BRK_CONT_NOT_IN_LOOP, leafNode->getToken().lineNum);
        }
        node->nextChild(); // jump CONTINUETK
    } else if (expect(*node->getChildIter(), SymbolEnum::RETURNTK)) {
        m_table.getCurrentScope().markHasReturn();
        auto leafNode = std::dynamic_pointer_cast<VNodeLeaf>(*node->getChildIter());
        node->nextChild(); // jump RETURNTK
        FuncItem* funcItem = m_table.getCurrentScope().getFuncItem();
        ValueTypeEnum type = funcItem->getReturnValueType();
        auto& funcName = funcItem->getName();
        int lineNum = leafNode->getToken().lineNum;
        if (expect(*node->getChildIter(), VNodeEnum::EXP)) {
            if (type == ValueTypeEnum::VOID_TYPE) {
                Logger::logError(ErrorType::VOID_FUNC_HAVE_RETURNED, lineNum, funcName);
            } else if (type == ValueTypeEnum::INT_TYPE) {
                exp<IntType>(*node->getChildIter());
            } else {
                exp<CharType>(*node->getChildIter());
            }
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
        std::vector<SymbolTableItem*> items;
        while (expect(*node->getChildIter(), SymbolEnum::COMMA)) {
            node->nextChild();                                    // jump ','
            items.push_back(exp<IntType>(*node->getChildIter())); // Char 是不能被%d打印的, 如果需要%c则按照顺序进行模板实例化即可
            node->nextChild();                                    // jump EXP
        }
        if (items.size() != count) {
            Logger::logError(ErrorType::PRINTF_UMATCHED, lineNum, std::to_string(items.size()), std::to_string(count));
        }
    } else if (expect(*node->getChildIter(), VNodeEnum::LVAL)) {
        auto lValItem = lVal(*node->getChildIter());
        if (lValItem) {
            node->nextChild(2); // jump lVal & =
            auto type = lValItem->getType()->getValueTypeEnum();
            SymbolTableItem* ret = nullptr;
            if (expect(*node->getChildIter(), SymbolEnum::GETINTTK)) {
                ret = MAKE_INT_VAR();
                // TODO: 生成将此通过getint获取值的代码
            } else {
                if (type == ValueTypeEnum::INT_TYPE) {
                    ret = exp<IntType>(*node->getChildIter());
                } else {
                    ret = exp<CharType>(*node->getChildIter());
                }
            }
            // TODO: 生成将暂存值存入左值的代码
        }
    } else if (expect(*node->getChildIter(), VNodeEnum::BLOCK)) {
        m_table.pushScope(BlockScopeType::NORMAL);
        block(*node->getChildIter());
        m_table.popScope();
    } else if (expect(*node->getChildIter(), VNodeEnum::EXP)) {
        exp<IntType>(*node->getChildIter());
    } else {
        Logger::logWarning("Empty statement or double semicolon.");
    }
}

SymbolTableItem* Visitor::lVal(std::shared_ptr<VNodeBase> node) {
    auto identNode = std::dynamic_pointer_cast<VNodeLeaf>(*node->getChildIter()); // lVal 的第一个子节点ident
    std::string identName = identNode->getToken().literal;
    int lineNum = identNode->getToken().lineNum;
    auto finded = m_table.findItem(identName);
    if (finded) {
        if (!finded->isChangble()) {
            Logger::logError(ErrorType::ASSIGN_TO_CONST, lineNum, identName);
        }
        if (!finded->getType()->isArray()) {
            return finded;
        } else {
            auto type = finded->getType()->getValueTypeEnum();
            std::vector<size_t> targetDims = getArrayItemDimensions(finded, type);
            SymbolTableItem* lVal = makeTempItem(type);

            std::vector<VarItem<IntType>*> pos;
            node->nextChild(1, false); // jump IDENT
            while (expect(*node->getChildIter(), SymbolEnum::LBRACK) && expect(*node->getChildIter(2), SymbolEnum::RBRACK)) {
                pos.push_back(dynamic_cast<VarItem<IntType>*>(exp<IntType>(*node->getChildIter(1))));
                if (!node->nextChild(3, false)) break; // jump '[pos]'
            }
            if (pos.size() != targetDims.size()) { // 如果维数不匹配则不是单个的数组元素，不能成为lVal
                Logger::logError("Can not convert a array to variable!");
                return nullptr;
            }
            return lVal;
        }
    } else {
        Logger::logError(ErrorType::UNDECL_IDENT, lineNum, identName);
        return nullptr;
    }
}

SymbolTableItem* Visitor::rVal(std::shared_ptr<VNodeBase> node) {
    auto identNode = std::dynamic_pointer_cast<VNodeLeaf>(*node->getChildIter()); // lVal 的第一个子节点ident
    std::string identName = identNode->getToken().literal;
    int lineNum = identNode->getToken().lineNum;
    auto finded = m_table.findItem(identName);
    if (finded) {
        if (!finded->getType()->isArray()) {
            return finded;
        } else {
            // 符号表中存储的数组的维数
            auto type = finded->getType()->getValueTypeEnum();
            std::vector<size_t> targetDims = getArrayItemDimensions(finded, type);
            SymbolTableItem* ret = nullptr;
            // 实际读取到的右值
            std::vector<VarItem<IntType>*> pos;
            node->nextChild(1, false); // jump IDENT
            while (expect(*node->getChildIter(), SymbolEnum::LBRACK) && expect(*node->getChildIter(2), SymbolEnum::RBRACK)) {
                pos.push_back(dynamic_cast<VarItem<IntType>*>(exp<IntType>(*node->getChildIter(1))));
                if (!node->nextChild(3, false)) break; // jump '[pos]'
            }
            int diff = targetDims.size() - pos.size();
            if (diff > 0) { // 维数不匹配，需要剪裁成部分数组
                std::vector<size_t> sliceDims(targetDims.begin(), targetDims.begin() + diff);
                ret = makeTempItem(type, true, std::move(sliceDims));
                // TODO: 生成sliceArray的代码

            } else if (diff == 0) { // 维数匹配，返回原数组的对应的元素
                ret = makeTempItem(type);
                // TODO: 生成返回一个元素的代码
            } else {
                Logger::logError("Variable dimension do not match!");
            }
            return ret;
        }
    } else {
        Logger::logError(ErrorType::UNDECL_IDENT, lineNum, identName);
        return nullptr;
    }
}

void Visitor::cond(std::shared_ptr<VNodeBase> node) {
    lOrExp<IntType>(*node->getChildIter());
}

template <typename Type>
SymbolTableItem* Visitor::lOrExp(std::shared_ptr<VNodeBase> node) {
    return nullptr;
}

std::vector<size_t> Visitor::getArrayItemDimensions(SymbolTableItem* item, ValueTypeEnum type) {
    if (item->getType()->getValueTypeEnum() == ValueTypeEnum::INT_TYPE) {
        return getArrayItemDimensions<IntType>(item);
    } else {
        return getArrayItemDimensions<CharType>(item);
    }
}

template <typename Type>
std::vector<size_t> Visitor::getArrayItemDimensions(SymbolTableItem* item) {
    if (item->isChangble()) {
        auto lValArray = dynamic_cast<VarItem<ArrayType<Type>>*>(item);
        return lValArray->getVarItem().getDimensions();
    } else {
        auto lValArray = dynamic_cast<ConstVarItem<ArrayType<Type>>*>(item);
        return lValArray->getConstVar().getDimensions();
    }
}

SymbolTableItem* Visitor::makeTempItem(ValueTypeEnum type, bool isArray, std::vector<size_t>&& dims) {
    SymbolTableItem* ret = nullptr;
    if (isArray) {
        switch (type) {
        case ValueTypeEnum::INT_TYPE: ret = MAKE_INT_ARRAY(dims); break;
        case ValueTypeEnum::CHAR_TYPE: ret = MAKE_CHAR_ARRAY(dims); break;
        case ValueTypeEnum::VOID_TYPE: break;
        }
    } else {
        switch (type) {
        case ValueTypeEnum::INT_TYPE: ret = MAKE_INT_VAR(); break;
        case ValueTypeEnum::CHAR_TYPE: ret = MAKE_CHAR_VAR(); break;
        case ValueTypeEnum::VOID_TYPE: break;
        }
    }
    return ret;
}