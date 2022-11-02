#include <ir/IR.h>

IndexMapper<Value> Value::s_valueMapper;
IndexMapper<BasicBlock> BasicBlock::s_bbMapper;
std::map<int, ConstValue*> ConstValue::POOL;

std::array<BasicBlock*, 2> BasicBlock::getSuccs() {
    auto& lastInst = m_insts.back();
    if (auto branchInst = dynamic_cast<BranchInst*>(lastInst.get())) {
        return {branchInst->m_left, branchInst->m_right};
    } else if (auto jumpInst = dynamic_cast<JumpInst*>(lastInst.get())) {
        return {jumpInst->m_next, nullptr};
    } else if (auto returnInst = dynamic_cast<ReturnInst*>(lastInst.get())) {
        return {nullptr, nullptr};
    } else {
        // Log error;
        return {nullptr, nullptr};
    }
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
    for (auto& var : m_globalVariables) {
        os << "@" << var.m_globalItem->getName() << " = ";
        if (var.m_globalItem->isChangble()) {
            os << "global ";
        } else {
            os << "constant ";
        }
        // type
        auto dims = getArrayItemDimensions(var.m_globalItem);
        printDimensions(os, dims);
        if (var.m_globalItem->hasInit()) {
            os << " ";
            var.m_globalItem->dumpSymbolItem(os, false);
        } else {
            os << "zeroinitializer";
        }
        os << std::endl;
    }
    os << std::endl;
    for (auto& func : m_funcs) {
        func->toCode(os);
    }
}

void Function::toCode(std::ostream& os) {
    Value::s_valueMapper.reset();
    BasicBlock::s_bbMapper.reset();
    std::string decl = m_isBuiltin ? "declare" : "define";
    std::string ret = m_funcItem->getReturnValueType() == ValueTypeEnum::INT_TYPE ? "i32" : "void";
    os << decl << " " << ret << " @";
    os << m_funcItem->getName() << "(";
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
    os << ") #0";
    if (m_isBuiltin) {
        os << std::endl;
    } else {
        os << " {" << std::endl;
        for (auto& bb : m_basicBlocks) { // 按顺序标号
            BasicBlock::s_bbMapper.get(bb.get());
        }
        for (auto& bb : m_basicBlocks) {
            int index = BasicBlock::s_bbMapper.get(bb.get());
            os << "_" << index << ": ; preds = ";
            for (int i = 0; i < bb->m_pred.size(); ++i) {
                if (i != 0) os << ", ";
                os << "%_" << BasicBlock::s_bbMapper.get(bb->m_pred[i]);
            }
            os << std::endl;
            for (auto& inst : bb->m_insts) {
                os << "\t";
                inst->toCode(os);
            }
        }
        os << "}" << std::endl
           << std::endl;
    }
}

void AllocaInst::toCode(std::ostream& os) {
    auto temp = Value::s_valueMapper.alloc();
    os << "%t" << temp << " = alloca ";
    auto dims = getArrayItemDimensions(m_sym);
    printDimensions(os, dims);
    os << ", align 4" << std::endl;
    os << "\t";
    printValue(os);
    os << " = getelementptr inbounds ";
    printDimensions(os, dims);
    os << ", ";
    printDimensions(os, dims);
    os << "* %t" << temp;
    if (dims.empty()) {
        os << ", i32 0" << std::endl;
    } else {
        os << ", i32 0, i32 0" << std::endl;
    }
};

void GetElementPtrInst::toCode(std::ostream& os) {
    os << "; getelementptr " << Value::s_valueMapper.get(this) << std::endl
       << "\t";
    int temp = Value::s_valueMapper.alloc();
    os << "%t" << temp << " = mul i32 ";
    m_index.value->printValue(os);
    os << ", " << m_multiplier << std::endl;
    os << "\t";
    printValue(os);
    os << " = getelementptr inbounds i32, i32* ";
    m_arr.value->printValue(os);
    os << ", i32 %t" << temp << std::endl;
}

void StoreInst::toCode(std::ostream& os) {
    os << "; store " << Value::s_valueMapper.get(this) << std::endl
       << "\t";
    // temp ptr
    int temp = Value::s_valueMapper.alloc();
    os << "%t" << temp << " = getelementptr inbounds i32, i32* ";
    m_arr.value->printValue(os);
    os << ", i32 ";
    m_index.value->printValue(os);
    os << std::endl;
    os << "\tstore i32 ";
    if (m_data.value) {
        m_data.value->printValue(os);
    }

    os << ", i32* %t" << temp << ", align 4" << std::endl;
}

void LoadInst::toCode(std::ostream& os) {
    // temp ptr
    int temp = Value::s_valueMapper.alloc();
    os << "%t" << temp << " = getelementptr inbounds i32, i32* ";
    m_arr.value->printValue(os);
    os << ", i32 ";
    m_index.value->printValue(os);
    os << std::endl;
    os << "\t";
    printValue(os);
    os << " = load i32, i32* %t" << temp << ", align 4" << std::endl;
}

void BinaryInst::toCode(std::ostream& os) {
    auto op_name = LLVM_OPS[(int)m_type];
    bool conversion = IRType::Lt <= m_type && m_type <= IRType::Ne;
    if (conversion) {
        int temp = Value::s_valueMapper.alloc();
        os << "%t" << temp << " = " << op_name << " i32 ";
        m_lhs.value->printValue(os);
        os << ", ";
        m_rhs.value->printValue(os);
        os << std::endl;
        os << "\t";
        printValue(os);
        os << " = zext i1 %t" << temp << " to i32" << std::endl;
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
    os << "br label %_" << BasicBlock::s_bbMapper.get(m_next) << std::endl;
}

void BranchInst::toCode(std::ostream& os) {
    // add comment
    os << "; if ";
    m_cond.value->printValue(os);
    os << " then _" << BasicBlock::s_bbMapper.get(m_left) << " else _"
       << BasicBlock::s_bbMapper.get(m_right) << std::endl;
    int temp = BasicBlock::s_bbMapper.alloc();
    os << "\t%t" << temp << " = icmp ne i32 ";
    m_cond.value->printValue(os);
    os << ", 0" << std::endl;
    os << "\tbr i1 %t" << temp << ", label %_" << BasicBlock::s_bbMapper.get(m_left) << ", label %_"
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
