#include <ir/IR.h>

IndexMapper<Value> Value::s_valueMapper;
IndexMapper<BasicBlock> BasicBlock::s_bbMapper;
std::map<int, ConstValue*> ConstValue::POOL;
std::map<std::string, Function*> Module::s_builtinFuncs;
std::set<Function*> Module::s_usedBuiltinFuncs;

std::array<BasicBlock*, 2> BasicBlock::getSuccs() {
    if (!m_insts.empty()) {
        auto& lastInst = m_insts.back();
        if (auto branchInst = dynamic_cast<BranchInst*>(lastInst.get())) {
            return {branchInst->m_left, branchInst->m_right};
        } else if (auto jumpInst = dynamic_cast<JumpInst*>(lastInst.get())) {
            return {jumpInst->m_next, nullptr};
        } else if (auto returnInst = dynamic_cast<ReturnInst*>(lastInst.get())) {
            return {nullptr, nullptr};
        }
    }
    return {nullptr, nullptr};
}

bool BasicBlock::valid() {
    auto& tail = m_insts.back();
    return (!m_insts.empty()) && (dynamic_cast<ReturnInst*>(tail.get()) || dynamic_cast<JumpInst*>(tail.get()) || dynamic_cast<BranchInst*>(tail.get()));
}

void printDimensions(std::ostream& os, std::vector<size_t>& dims) {
    if (dims.empty()) {
        os << "i32";
    } else {
        size_t dimsMul = 1;
        for (auto& dim : dims) {
            dimsMul *= dim;
        }
        os << "[" << dimsMul << " x i32]";
    }
}

void Module::toCode(std::ostream& os) {
    for (auto& builtinFunc : s_builtinFuncs) {
        builtinFunc.second->toCode(os);
    }
    if (!s_usedBuiltinFuncs.empty()) {
        os << std::endl;
    }
    for (auto& var : m_globalVariables) {
        os << "@" << var.get()->m_globalItem->getName() << " = ";
        if (var.get()->m_globalItem->isChangble()) {
            os << "global ";
        } else {
            os << "constant ";
        }
        // type
        auto dims = getArrayItemDimensions(var.get()->m_globalItem);
        printDimensions(os, dims);
        if (var.get()->m_globalItem->hasInit()) {
            os << " ";
            var.get()->m_globalItem->dumpSymbolItem(os, false);
        } else {
            os << " zeroinitializer";
        }
        os << std::endl;
    }
    if (!m_globalVariables.empty()) {
        os << std::endl;
    }
    for (auto& str : m_strVariables) {
        str->printValue(os);
        os << " = constant ";
        str->printStrType(os);
        os << " ";
        str->printString(os);
        os << std::endl;
    }
    if (!m_strVariables.empty()) {
        os << std::endl;
    }
    for (auto& func : m_funcs) {
        for (auto& bb : func->m_basicBlocks) {
            bb->getPreds().clear();
        }
        for (auto& bb : func->m_basicBlocks) {
            // bb->getPreds().clear();
            for (auto* x : bb->getSuccs()) {
                if (x) x->getPreds().push_back(bb.get());
            }
        }
        func->toCode(os);
    }
}

Function* Module::getFunc(FuncItem* funcItem) {
    for (auto& func : m_funcs) {
        if (func->getFuncItem() == funcItem) {
            return func.get();
        }
    }
    return nullptr;
}

Function* Module::getBuiltinFunc(const std::string& funcName) {
    Function* func = s_builtinFuncs.at(funcName);
    s_usedBuiltinFuncs.insert(func);
    return func;
}

void Function::toCode(std::ostream& os) {
    Value::s_valueMapper.reset();
    BasicBlock::s_bbMapper.reset();
    std::string decl = m_isBuiltin ? "declare" : "define";
    std::string ret;
    if (m_isBuiltin) {
        ret = m_retType;
    } else {
        ret = m_funcItem->getReturnValueType() == ValueTypeEnum::INT_TYPE ? "i32" : "void";
    }
    os << decl << " " << ret << " @";
    if (m_isBuiltin) {
        os << m_builtinName;
    } else {
        os << m_funcItem->getName();
    }
    os << "(";
    if (m_isBuiltin) {
        os << m_builtinArgType << " ";
    } else {
        for (auto p : m_funcItem->getParams()) {
            auto dims = getArrayItemDimensions(p);
            if (dims.size() == 0) {
                // simple case
                os << "i32 ";
            } else {
                // array arg
                os << "i32 * ";
            }
            os << "%" << p->getName();
            if (p != m_funcItem->getParams().back()) {
                // not last element
                os << ", ";
            }
        }
    }

    os << ") #0";
    if (m_isBuiltin) {
        os << std::endl;
    } else {
        os << " {" << std::endl;
        for (auto& bb : m_basicBlocks) { // 按顺序标号
            BasicBlock::s_bbMapper.get(bb.get());
        }
        auto& lastBlock = m_basicBlocks.back();
        if (!lastBlock->valid()) {
            if (m_funcItem->getReturnValueType() == ValueTypeEnum::VOID_TYPE) {
                lastBlock->pushBackInst(new ReturnInst(nullptr));
            } else {
                lastBlock->pushBackInst(new ReturnInst(ConstValue::get(0)));
            }
        }
        for (auto& bb : m_basicBlocks) {
            int index = BasicBlock::s_bbMapper.get(bb.get());
            os << "_b" << index << ": ; preds = ";
            for (int i = 0; i < bb->m_pred.size(); ++i) {
                if (i != 0) os << ", ";
                os << "%_b" << BasicBlock::s_bbMapper.get(bb->m_pred[i]);
            }
            os << std::endl;
            for (auto& inst : bb->m_insts) {
                os << "\t";
                inst->toCode(os);
            }
        }
        os << "}" << std::endl;
        if (!m_isBuiltin) {
            os << std::endl;
        }
    }
}

void AllocaInst::toCode(std::ostream& os) {
    auto temp = Value::s_valueMapper.alloc();
    os << "%_t" << temp << " = alloca ";
    auto dims = getArrayItemDimensions(m_sym);
    printDimensions(os, dims);
    os << ", align 4" << std::endl;
    os << "\t";
    printValue(os);
    os << " = getelementptr inbounds ";
    printDimensions(os, dims);
    os << ", ";
    printDimensions(os, dims);
    os << "* %_t" << temp;
    if (dims.empty()) {
        os << ", i32 0" << std::endl;
    } else {
        os << ", i32 0, i32 0" << std::endl;
    }
};

void GetElementPtrInst::toCode(std::ostream& os) {
    os << "; getelementptr " << Value::s_valueMapper.get(this) << std::endl
       << "\t";
    if (auto index = dynamic_cast<ConstValue*>(m_index.value)) {
        int res = index->getImm() * m_multiplier;
        printValue(os);
        os << " = getelementptr inbounds ";
        if (m_arr.value) {
            if (m_arr.value->isGlob()) {
                auto globItem = dynamic_cast<GlobalVariable*>(m_arr.value)->getGlobalItem();
                auto dims = getArrayItemDimensions(globItem);
                printDimensions(os, dims);
                os << ", ";
                printDimensions(os, dims);
                os << "* ";
                if (!dims.empty()) {
                    m_arr.value->printValue(os);
                    os << ", i32 0, i32 0" << std::endl;
                    return;
                }
            } else {
                os << "i32, i32* ";
            }
            m_arr.value->printValue(os);
        }
        os << ", i32 " << res << std::endl;
    } else {
        int temp = Value::s_valueMapper.alloc();
        os << "%_t" << temp << " = mul i32 ";
        m_index.value->printValue(os);
        os << ", " << m_multiplier << std::endl;
        os << "\t";
        printValue(os);
        os << " = getelementptr inbounds i32, i32* ";
        m_arr.value->printValue(os);
        os << ", i32 %_t" << temp << std::endl;
    }
}

void StoreInst::toCode(std::ostream& os) {
    os << "; store " << Value::s_valueMapper.get(this) << std::endl
       << "\t";
    // temp ptr
    if (dynamic_cast<ConstValue*>(m_index.value)->getImm() != 0) {
        int temp = Value::s_valueMapper.alloc();
        os << "%_t" << temp << " = getelementptr inbounds i32, i32* ";
        m_arr.value->printValue(os);
        os << ", i32 ";
        m_index.value->printValue(os);
        os << std::endl;
        os << "\tstore i32 ";
        if (m_data.value) {
            m_data.value->printValue(os);
        }
        os << ", i32* %_t" << temp << ", align 4" << std::endl;
    } else {
        os << "store i32 ";
        if (m_data.value) {
            m_data.value->printValue(os);
        }
        os << ", i32* ";
        m_arr.value->printValue(os);
        os << ", align 4" << std::endl;
    }
}

void LoadInst::toCode(std::ostream& os) {
    // temp ptr
    if (dynamic_cast<ConstValue*>(m_index.value)->getImm() != 0) {
        int temp = Value::s_valueMapper.alloc();
        os << "%_t" << temp << " = getelementptr inbounds i32, i32* ";
        if (m_arr.value) {
            m_arr.value->printValue(os);
        }
        os << ", i32 ";
        m_index.value->printValue(os);
        os << std::endl;
        os << "\t";
        printValue(os);
        os << " = load i32, i32* %_t" << temp << ", align 4" << std::endl;
    } else {
        printValue(os);
        os << " = load i32, i32* ";
        if (m_arr.value) {
            m_arr.value->printValue(os);
        }
        os << ", align 4" << std::endl;
    }
}

void BinaryInst::toCode(std::ostream& os) {
    auto op_name = LLVM_OPS[(int)m_type];
    bool conversion = IRType::Lt <= m_type && m_type <= IRType::Ne;
    if (conversion) {
        int temp = Value::s_valueMapper.alloc();
        os << "%_t" << temp << " = " << op_name << " i32 ";
        m_lhs.value->printValue(os);
        os << ", ";
        m_rhs.value->printValue(os);
        os << std::endl;
        os << "\t";
        printValue(os);
        os << " = zext i1 %_t" << temp << " to i32" << std::endl;
    } else if (m_type == IRType::Rsb) {
        printValue(os);
        os << " = sub i32 ";
        m_rhs.value->printValue(os);
        os << ", ";
        m_lhs.value->printValue(os);
        os << std::endl;
    } else {
        printValue(os);
        os << " = " << op_name << " i32 ";
        m_lhs.value->printValue(os);
        os << ", ";
        m_rhs.value->printValue(os);
        os << std::endl;
    }
}

void JumpInst::toCode(std::ostream& os) {
    os << "br label %_b" << BasicBlock::s_bbMapper.get(m_next) << std::endl;
}

void BranchInst::toCode(std::ostream& os) {
    // add comment
    os << "; if ";
    m_cond.value->printValue(os);
    os << " then _b" << BasicBlock::s_bbMapper.get(m_left) << " else _b"
       << BasicBlock::s_bbMapper.get(m_right) << std::endl;
    int temp = Value::s_valueMapper.alloc();
    os << "\t%_t" << temp << " = icmp ne i32 ";
    m_cond.value->printValue(os);
    os << ", 0" << std::endl;
    os << "\tbr i1 %_t" << temp << ", label %_b" << BasicBlock::s_bbMapper.get(m_left) << ", label %_b"
       << BasicBlock::s_bbMapper.get(m_right) << std::endl;
}

void ReturnInst::toCode(std::ostream& os) {
    if (m_ret.value) {
        os << "ret i32 ";
        m_ret.value->printValue(os);
        os << std::endl;
    } else {
        os << "ret void" << std::endl;
    }
}

void CallInst::toCode(std::ostream& os) {
    if (m_func->m_isBuiltin) {
        if (m_func->m_retType == "void") {
            os << "call void";
        } else {
            printValue(os);
            os << " = call " << m_func->m_retType;
        }
        os << " @" << m_func->m_builtinName << "(";
        if (!m_args.empty()) {
            os << m_func->m_builtinArgType << " ";
            m_args[0].value->printValue(os);
        }
        os << ")" << std::endl;
    } else {
        FuncItem* callee = m_func->getFuncItem();
        if (callee->getReturnValueType() == ValueTypeEnum::INT_TYPE) {
            printValue(os);
            os << " = call i32";
        } else {
            os << "call void";
        }
        os << " @" << callee->getName() << "(";
        for (int i = 0; i < m_args.size(); i++) {
            // type
            auto param = callee->getParams()[i];
            auto dims = getArrayItemDimensions(param);
            if (dims.empty()) {
                // simple
                os << "i32 ";
            } else {
                // array param
                os << "i32 * ";
            }
            // arg
            m_args[i].value->printValue(os);
            if (i + 1 < m_args.size()) {
                // not last element
                os << ", ";
            }
        }
        os << ")" << std::endl;
    }
}

void PhiInst::toCode(std::ostream& os) {
    printValue(os);
    os << " = phi i32 ";
    for (int i = 0; i < m_incomingValues.size(); ++i) {
        if (i != 0) os << ", ";
        os << "[";
        m_incomingValues[i].value->printValue(os);
        os << ", %_b" << BasicBlock::s_bbMapper.get(m_atBlock->getPreds()[i]) << "]";
    }
    os << std::endl;
}

void PrintInst::printPutInt(const Use& arg, std::ostream& os) {
    os << "call void @putint(i32 ";
    arg.value->printValue(os);
    os << ")" << std::endl;
}

void PrintInst::printPutStr(StringVariable* strPart, std::ostream& os) {
    auto temp = Value::s_valueMapper.alloc();
    os << "%_t" << temp << " = getelementptr inbounds ";
    strPart->printStrType(os);
    os << ", ";
    strPart->printStrType(os);
    os << "* ";
    strPart->printValue(os);
    os << ", i32 0, i32 0" << std::endl;
    os << "\tcall void @putstr(i8* "
       << "%_t" << temp << ")" << std::endl;
}

void PrintInst::toCode(std::ostream& os) {
    int partsNum = m_strParts.size();
    if (partsNum == 0) { // 只有 %d
        Module::getBuiltinFunc("putint");
        bool flag = true;
        for (auto argIt = m_args.begin(); argIt != m_args.end(); argIt++) {
            if (argIt != m_args.begin()) {
                os << "\t";
            }
            printPutInt(*argIt, os);
        }
    } else if (partsNum == 1) { // 只有 "str"
        Module::getBuiltinFunc("putstr");
        printPutStr(m_strParts[0], os);
    } else {
        Module::getBuiltinFunc("putint");
        Module::getBuiltinFunc("putstr");
        int argCnt = 0;
        for (auto it = m_strParts.begin(); it != m_strParts.end(); it++) {
            if (it != m_strParts.begin()) {
                os << "\t";
            }
            if (*it) {
                printPutStr(*it, os);
            } else {
                printPutInt(m_args[argCnt++], os);
            }
        }
    }
}

void StringVariable::printStrType(std::ostream& os) {
    os << "[" << m_len << " x i8]";
}

void StringVariable::printString(std::ostream& os) {
    os << "c\"" << m_str << "\"";
}