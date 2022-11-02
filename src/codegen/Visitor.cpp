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

Visitor::Visitor(std::shared_ptr<VNodeBase> astRoot, SymbolTable& table, CodeContext& ctx) :
    m_astRoot(astRoot), m_table(table), m_ctx(ctx) {}

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
    return calConstExp<Type>(*node->getChildIter()).first;
}
template <typename Type>
SymbolTableItem* Visitor::exp(std::shared_ptr<VNodeBase> node) {
    return addExp<Type>(*node->getChildIter());
}

template <typename Type>
SymbolTableItem* Visitor::addExp(std::shared_ptr<VNodeBase> node) {
    auto res = calConstExp<Type>(node);
    if (res.second) {
        auto item = m_table.makeItem<ConstVarItem<Type>>(res.first);
        item->setIrValue(ConstValue::get(res.first));
        return item;
    } else {
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
}

template <typename Type>
SymbolTableItem* Visitor::mulExp(std::shared_ptr<VNodeBase> node) {
    auto res = calConstExp<Type>(node);
    if (res.second) {
        auto item = m_table.makeItem<ConstVarItem<Type>>(res.first);
        item->setIrValue(ConstValue::get(res.first));
        return item;
    } else {
        node->resetIter();
        if (expect(*node->getChildIter(), VNodeEnum::UNARYEXP)) {
            return unaryExp<Type>(*node->getChildIter());
        } else {
            auto mul = mulExp<Type>(*node->getChildIter());
            node->nextChild();
            SymbolEnum op = (*node->getChildIter())->getSymbol(); // get symbol of plus or minus
            node->nextChild();
            auto unary = unaryExp<Type>(*node->getChildIter());
            auto ret = m_table.makeItem<VarItem<Type>>({});
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
}

template <typename Type>
SymbolTableItem* Visitor::unaryExp(std::shared_ptr<VNodeBase> node) {
    auto res = calConstExp<Type>(node);
    if (res.second) {
        auto item = m_table.makeItem<ConstVarItem<Type>>(res.first);
        item->setIrValue(ConstValue::get(res.first));
        return item;
    } else {
        node->resetIter();
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
                return nullptr;
            } else {
                if (expect(*node->getChildIter(), VNodeEnum::FUNCRPARAMS)) {
                    realParams = funcRParams(*node->getChildIter(), func, lineNum); // 传入func
                } else {
                    auto expectParams = func->getParams().size();
                    if (expectParams != 0) {
                        Logger::logError(ErrorType::PARAMS_NUM_NOT_MATCH, lineNum, "0", std::to_string(expectParams));
                    }
                }
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
        auto realArr = exp<Type>(node);
        if (realArr && realArr->getType()->getValueTypeEnum() != ValueTypeEnum::VOID_TYPE) {
            auto formalDims = getArrayItemDimensions<Type>(formalParam);
            if (checkConvertiable<ArrayType<Type>>(realArr)) {
                auto realDims = getArrayItemDimensions<Type>(realArr);
                ret = (realDims.size() == formalDims.size()) ? realArr : nullptr;
            }
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
    auto res = calConstExp<Type>(node);
    if (res.second) {
        return m_table.makeItem<ConstVarItem<Type>>(res.first);
    } else {
        node->resetIter();
        if (expect(*node->getChildIter(), SymbolEnum::LPARENT)) {
            node->nextChild();
            return exp<Type>(*node->getChildIter());
        } else if (expect(*node->getChildIter(), VNodeEnum::LVAL)) {
            // 检查rVal返回的值类型是否与指定Type相同，如果不同给出警告并转换，不可转换则报错;
            return rVal<Type>(*node->getChildIter());
        } else {
            return number<Type>(*node->getChildIter());
        }
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
ConstVarItem<Type>* Visitor::number(std::shared_ptr<VNodeBase> node) {
    return m_table.makeItem<ConstVarItem<Type>>({});
}

template <>
ConstVarItem<IntType>* Visitor::number(std::shared_ptr<VNodeBase> node) {
    auto leafNode = std::dynamic_pointer_cast<VNodeLeaf>(*node->getChildIter());
    auto lineNum = leafNode->getToken().lineNum;
    auto value = static_cast<typename IntType::InternalType>(leafNode->getToken().value);
    auto number = m_table.makeItem<ConstVarItem<IntType>>(value);
    return number;
}

template <>
ConstVarItem<CharType>* Visitor::number(std::shared_ptr<VNodeBase> node) {
    auto leafNode = std::dynamic_pointer_cast<VNodeLeaf>(*node->getChildIter());
    auto value = static_cast<typename CharType::InternalType>(leafNode->getToken().value);
    auto number = m_table.makeItem<ConstVarItem<CharType>>(value);
    return number;
}

template <typename Type>
std::pair<typename Type::InternalType, bool> Visitor::calConstExp(std::shared_ptr<VNodeBase> node) {
    if (node->getType() == VType::VT) {
        if (expect(node, SymbolEnum::INTCON)) {
            return {std::dynamic_pointer_cast<VNodeLeaf>(node)->getToken().value, true};
        }
        return {std::dynamic_pointer_cast<VNodeLeaf>(node)->getToken().value, true};
    } else {
        switch (node->getNodeEnum()) {
        case VNodeEnum::ADDEXP:
            if (node->getChildrenNum() == 1) {
                return calConstExp<Type>(*node->getChildIter());
            } else {
                auto res1 = calConstExp<Type>(*node->getChildIter());
                auto res2 = calConstExp<Type>(*node->getChildIter(2));
                if (res1.second && res2.second) {
                    if ((*node->getChildIter(1))->getSymbol() == SymbolEnum::PLUS) {
                        return {res1.first + res2.first, true};
                    } else {
                        return {res1.first - res2.first, true};
                    }
                }
            }
            break;
        case VNodeEnum::MULEXP:
            if (node->getChildrenNum() == 1) {
                return calConstExp<Type>(*node->getChildIter());
            } else {
                auto res1 = calConstExp<Type>(*node->getChildIter());
                auto res2 = calConstExp<Type>(*node->getChildIter(2));
                if (res1.second && res2.second) {
                    if ((*node->getChildIter(1))->getSymbol() == SymbolEnum::MULT) {
                        return {res1.first * res2.first, true};
                    } else if ((*node->getChildIter(1))->getSymbol() == SymbolEnum::DIV) {
                        return {res1.first / res2.first, true};
                    } else {
                        return {res1.first % res2.first, true};
                    }
                }
            }
            break;
        case VNodeEnum::UNARYEXP:
            if (expect(*node->getChildIter(), SymbolEnum::IDENFR)) { // func
                return {0, false};
            } else if (expect(*node->getChildIter(), VNodeEnum::PRIMARYEXP)) {
                return calConstExp<Type>(*node->getChildIter());
            } else {
                auto symbol = (*(*node->getChildIter())->getChildIter())->getSymbol();
                auto res = calConstExp<Type>(*node->getChildIter(1));
                if (symbol == SymbolEnum::PLUS) {
                    return res;
                } else if (symbol == SymbolEnum::MINU) {
                    res.first = -res.first;
                    return res;
                } else {
                    return {!res.first, false};
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
                auto constArrayItem = dynamic_cast<ConstVarItem<ArrayType<Type>>*>(item);
                if (constVarItem || constArrayItem) {
                    if (constVarItem) {
                        return {constVarItem->getConstVar(), true};
                    }
                    if (constArrayItem) {
                        std::vector<size_t> dims;
                        node->nextChild();
                        while (expect(*node->getChildIter(), SymbolEnum::LBRACK) && expect(*node->getChildIter(2), SymbolEnum::RBRACK)) {
                            auto res = exp<Type>(*node->getChildIter(1));
                            if (auto dim = dynamic_cast<ConstVarItem<Type>*>(res)) {
                                if (dim->getConstVar() >= 0) {
                                    dims.push_back(static_cast<size_t>(dim->getConstVar()));
                                } else {
                                    Logger::logError("Use dimension as negative size!");
                                }
                            } else {
                                return {0, false};
                            }
                            if (!node->nextChild(3, false)) break; // jump '[dim]'
                        }
                        auto val = constArrayItem->getConstVar()[std::move(dims)];
                        return {val, true};
                    }
                } else {
                    return {0, false};
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
    return {0, false};
}

template <typename Type>
typename Type::InternalType Visitor::constInitVal(std::shared_ptr<VNodeBase> node, std::vector<size_t>& dims, int level) {
    return constExp<Type>(*node->getChildIter());
};

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
        typename Type::InternalType val = constInitVal<Type>(node, dims, level + 1);
        values.insert(val);
    }
    values.setDimensions(dims);
    return values;
};

template <typename Type>
typename Type::InternalItem Visitor::initVal(std::shared_ptr<VNodeBase> node, std::vector<size_t>& dims, int level) {
    return exp<Type>(*node->getChildIter());
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
        typename Type::InternalItem val = initVal<Type>(node, dims, level + 1);
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
    typename Type::InternalType var;
    MultiFlatArray<typename Type::InternalType> varArray;
    bool notArray = dims.size() == 0;
    if (notArray) {
        var = constInitVal<Type>(*node->getChildIter(), dims, 0);
        res = m_table.insertItem<ConstVarItem<Type>>(identName, var);

    } else {
        varArray = constInitValArray<Type>(*node->getChildIter(), dims, 0);
        res = m_table.insertItem<ConstVarItem<ArrayType<Type>>>(identName, varArray);
    }

    /*---------------------------------codegen------------------------------------*/
    if (m_table.getCurrentScope().getType() != BlockScopeType::GLOBAL) {
        auto inst = m_ctx.basicBlock->pushBackInst(new AllocaInst(res.first));
        if (notArray) {
            m_ctx.basicBlock->pushBackInst(new StoreInst(res.first, inst, ConstValue::get(var), ConstValue::get(0)));
        } else {
            for (auto& var : varArray.getValues()) {
                m_ctx.basicBlock->pushBackInst(new StoreInst(res.first, inst, ConstValue::get(var), ConstValue::get(0)));
            }
        }
    } else {
        m_ctx.module.addGlobalVar(res.first);
    }
    /*----------------------------------------------------------------------------*/

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
        MultiFlatArray<typename Type::InternalType> varArray;
        typename Type::InternalType var;
        bool hasInit = false;
        if (expect(*node->getChildIter(), SymbolEnum::ASSIGN)) {
            node->nextChild();
            hasInit = true;
            if (dims.size() == 0) {
                var = constInitVal<Type>(*node->getChildIter(), dims, 0);
            } else {
                varArray = constInitValArray<Type>(*node->getChildIter(), dims, 0);
            }
        } else {
            varArray.setDimensions(dims);
        }
        if (dims.size() == 0) {
            res = m_table.insertItem<VarItem<Type>>(identName, {nullptr, var, hasInit});
        } else {
            res = m_table.insertItem<VarItem<ArrayType<Type>>>(identName, {{{.values = {}, .dimensions = varArray.getDimensions()}}, varArray, hasInit});
        }
        /*---------------------------------codegen------------------------------------*/
        m_ctx.module.addGlobalVar(res.first);
        /*----------------------------------------------------------------------------*/
        if (!res.second) {
            Logger::logError(ErrorType::REDEF_IDENT, lineNum, identName);
        }
    } else {
        MultiFlatArray<SymbolTableItem*> itemArray;
        SymbolTableItem* item;
        bool hasInit = false;
        bool notArray = dims.size() == 0;
        if (expect(*node->getChildIter(), SymbolEnum::ASSIGN)) {
            node->nextChild();
            hasInit = true;
            if (notArray) {
                item = initVal<Type>(*node->getChildIter(), dims, 0);
            } else {
                itemArray = initValArray<Type>(*node->getChildIter(), dims, 0);
            }
        } else {
            itemArray.setDimensions(dims);
        }
        if (notArray) {
            res = m_table.insertItem<VarItem<Type>>(identName, {item, 0, hasInit});
        } else {
            res = m_table.insertItem<VarItem<ArrayType<Type>>>(identName, {itemArray, {}, hasInit});
        }
        /*---------------------------------codegen------------------------------------*/
        auto inst = m_ctx.basicBlock->pushBackInst(new AllocaInst(res.first));
        if (hasInit) {
            if (notArray) {
                m_ctx.basicBlock->pushBackInst(new StoreInst(res.first, inst, item->getIrValue(), ConstValue::get(0)));
            } else {
                for (auto& item : itemArray.getValues()) {
                    m_ctx.basicBlock->pushBackInst(new StoreInst(res.first, inst, item->getIrValue(), ConstValue::get(0)));
                }
            }
        }
        /*----------------------------------------------------------------------------*/
        if (!res.second) {
            Logger::logError(ErrorType::REDEF_IDENT, lineNum, identName);
        }
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
    auto res = m_table.insertFunc(identName, ValueTypeEnum::INT_TYPE);
    if (!res.second) {
        Logger::logError(ErrorType::REDEF_IDENT, lineNum, identName);
    }
    node->nextChild(2); // jump MAINTK & '('
    m_table.pushScope(BlockScopeType::FUNC);
    m_table.getCurrentScope().setFuncItem(res.first);
    /*---------------------------------codegen------------------------------------*/
    m_ctx.function = m_ctx.module.addFunc(new Function(res.first));
    m_ctx.basicBlock = m_ctx.function->pushBackBasicBlock(new BasicBlock());
    /*----------------------------------------------------------------------------*/
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
    auto res = m_table.insertFunc(identName, retType);
    if (!res.second) {
        Logger::logError(ErrorType::REDEF_IDENT, lineNum, identName);
        return;
    }
    node->nextChild(2); // jump IDENT '('
    m_table.pushScope(BlockScopeType::FUNC);
    m_table.getCurrentScope().setFuncItem(res.first);
    /*---------------------------------codegen------------------------------------*/
    m_ctx.function = m_ctx.module.addFunc(new Function(res.first));
    m_ctx.basicBlock = m_ctx.function->pushBackBasicBlock(new BasicBlock());
    /*----------------------------------------------------------------------------*/
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
        if (!node->nextChild(1, false)) break;
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
            auto res = m_table.insertItem<VarItem<IntType>>(identName, {nullptr, 0});
            ret = res.first;
            valid = res.second;
        } else {
            auto res = m_table.insertItem<VarItem<ArrayType<IntType>>>(identName, {{{{}, dims}}, {{{}, dims}}, false});
            ret = res.first;
            valid = res.second;
        }
    } else if (type == ValueTypeEnum::CHAR_TYPE) {
        if (dims.size() == 0) {
            auto res = m_table.insertItem<VarItem<CharType>>(identName, {nullptr, 0});
            ret = res.first;
            valid = res.second;
        } else {
            auto res = m_table.insertItem<VarItem<ArrayType<CharType>>>(identName, {{{{}, dims}}, {{{}, dims}}, false});
            ret = res.first;
            valid = res.second;
        }
    }
    if (!valid) {
        Logger::logError(ErrorType::REDEF_IDENT, lineNum, identName);
    }
    ret->setParam();
    return ret;
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
        node->nextChild(2); // jump COND ')'
        m_table.pushScope(BlockScopeType::BRANCH);
        stmt(*node->getChildIter());
        m_table.popScope();
        node->nextChild(1, false); // jump STMT
        if (expect(*node->getChildIter(), SymbolEnum::ELSETK)) {
            node->nextChild(); // jump ELSETK
            m_table.pushScope(BlockScopeType::BRANCH);
            stmt(*node->getChildIter());
            m_table.popScope();
            node->nextChild(1, false); // jump STMT
        }
    } else if (expect(*node->getChildIter(), SymbolEnum::WHILETK)) {
        node->nextChild(2); // jump WHILE & '('
        cond(*node->getChildIter());
        node->nextChild(2);
        m_table.pushScope(BlockScopeType::LOOP);
        stmt(*node->getChildIter());
        m_table.popScope();
        node->nextChild(1, false); // jump STMT
    } else if (expect(*node->getChildIter(), SymbolEnum::BREAKTK)) {
        auto leafNode = std::dynamic_pointer_cast<VNodeLeaf>(*node->getChildIter());
        if (!m_table.getCurrentScope().isSubLoopScope()) {
            Logger::logError(ErrorType::BRK_CONT_NOT_IN_LOOP, leafNode->getToken().lineNum);
        }
        node->nextChild(); // jump BREAKTK
    } else if (expect(*node->getChildIter(), SymbolEnum::CONTINUETK)) {
        auto leafNode = std::dynamic_pointer_cast<VNodeLeaf>(*node->getChildIter());
        if (!m_table.getCurrentScope().isSubLoopScope()) {
            Logger::logError(ErrorType::BRK_CONT_NOT_IN_LOOP, leafNode->getToken().lineNum);
        }
        node->nextChild(); // jump CONTINUETK
    } else if (expect(*node->getChildIter(), SymbolEnum::RETURNTK)) {
        m_table.getCurrentScope().markHasReturn();
        auto leafNode = std::dynamic_pointer_cast<VNodeLeaf>(*node->getChildIter());
        node->nextChild(); // jump RETURNTK
        FuncItem* funcItem = m_table.getCurrentScope().getFuncItem();
        if (funcItem) {
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
            std::vector<size_t> targetDims = getArrayItemDimensions(finded);
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

template <typename Type>
SymbolTableItem* Visitor::rVal(std::shared_ptr<VNodeBase> node) {
    auto res = calConstExp<Type>(node);
    if (res.second) {
        auto item = m_table.makeItem<ConstVarItem<Type>>(res.first);
        item->setIrValue(ConstValue::get(res.first));
        return item;
    } else {
        node->resetIter();
        auto identNode = std::dynamic_pointer_cast<VNodeLeaf>(*node->getChildIter()); // lVal 的第一个子节点ident
        std::string identName = identNode->getToken().literal;
        int lineNum = identNode->getToken().lineNum;
        auto finded = m_table.findItem(identName);
        if (finded) {
            auto type = finded->getType()->getValueTypeEnum();
            bool findedIsArray = finded->getType()->isArray();
            if (!findedIsArray) {
                return finded;
            } else {
                // 符号表中存储的数组的维数

                std::vector<size_t> targetDims = getArrayItemDimensions(finded);
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
}

void Visitor::cond(std::shared_ptr<VNodeBase> node) {
    lOrExp<IntType>(*node->getChildIter());
}

template <typename Type>
void Visitor::lOrExp(std::shared_ptr<VNodeBase> node) {
    if (expect(*node->getChildIter(), VNodeEnum::LANDEXP)) {
        lAndExp<Type>(*node->getChildIter());
    } else {
        lOrExp<Type>(*node->getChildIter());
        node->nextChild();
        SymbolEnum op = (*node->getChildIter())->getSymbol(); // get symbol of or
        node->nextChild();
        lAndExp<Type>(*node->getChildIter());
        if (op == SymbolEnum::OR) {
            // 生成或的代码
        } else {
            DBG_ERROR("lOr expression only accept '||'!");
        }
    }
}

template <typename Type>
void Visitor::lAndExp(std::shared_ptr<VNodeBase> node) {
    if (expect(*node->getChildIter(), VNodeEnum::EQEXP)) {
        auto eq = eqExp<Type>(*node->getChildIter());
        // TODO: 生成condition的代码
    } else {
        lAndExp<Type>(*node->getChildIter());
        node->nextChild();
        SymbolEnum op = (*node->getChildIter())->getSymbol(); // get symbol and
        node->nextChild();
        auto eq = eqExp<Type>(*node->getChildIter());
        // TODO: 生成condition的代码
        if (op == SymbolEnum::AND) {
            // 生成且的代码
        } else {
            DBG_ERROR("lAnd expression only accept '&&'!");
        }
    }
}

template <typename Type>
SymbolTableItem* Visitor::eqExp(std::shared_ptr<VNodeBase> node) {
    if (expect(*node->getChildIter(), VNodeEnum::RELEXP)) {
        return relExp<Type>(*node->getChildIter());
    } else {
        auto eq = eqExp<Type>(*node->getChildIter());
        node->nextChild();
        SymbolEnum op = (*node->getChildIter())->getSymbol(); // get symbol of eql or neq
        node->nextChild();
        auto rel = relExp<Type>(*node->getChildIter());
        auto ret = MAKE_VAR();
        if (op == SymbolEnum::EQL) {
            // 生成相等的代码
        } else if (op == SymbolEnum::NEQ) {
            // 生成不等的代码
        } else {
            DBG_ERROR("Equal expression only accept '==' & '!='!");
            return nullptr;
        }
        return ret;
    }
}

template <typename Type>
SymbolTableItem* Visitor::relExp(std::shared_ptr<VNodeBase> node) {
    if (expect(*node->getChildIter(), VNodeEnum::ADDEXP)) {
        return addExp<Type>(*node->getChildIter());
    } else {
        auto rel = relExp<Type>(*node->getChildIter());
        node->nextChild();
        SymbolEnum op = (*node->getChildIter())->getSymbol(); // get symbol of less and great
        node->nextChild();
        auto add = addExp<Type>(*node->getChildIter());
        auto ret = MAKE_VAR();
        if (op == SymbolEnum::LSS) {
            // 生成小于的代码
        } else if (op == SymbolEnum::GRE) {
            // 生成大于的代码
        } else if (op == SymbolEnum::LEQ) {
            // 生成小于等于的代码
        } else if (op == SymbolEnum::GEQ) {
            // 生成大于等于的代码
        } else {
            DBG_ERROR("Relation expression only accept '<' & '>' & '<=' & '>='!");
            return nullptr;
        }
        return ret;
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