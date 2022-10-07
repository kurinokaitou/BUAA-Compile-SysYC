#include <symbol/BlockScope.h>
#include <symbol/SymbolTable.h>
#include <Log.h>
BlockScope::BlockScope(SymbolTable& table, BlockScopeType type, int level, BlockScopeHandle parent) :
    m_symbolTable(table), m_type(type), m_level(level), m_parentHandle(parent) {
}

void BlockScope::addChildScope(BlockScopeHandle handle) {
    m_childrenHandle.push_back(handle);
}

SymbolTableItem* BlockScope::findItem(const std::string& name) {
    // 在当前作用域中寻找
    auto item = std::find_if(m_symbols.begin(), m_symbols.end(), [&name](const std::unique_ptr<SymbolTableItem>& symbol) {
        return symbol->getName() == name;
    });
    if (item != m_symbols.end()) {
        return item->get();
    } else {
        if (m_type == BlockScopeType::FUNC) {
            // 如果是函数作用域，在形参中寻找
            auto paramItem = std::find_if(m_paramItems.begin(), m_paramItems.end(), [&name](SymbolTableItem* param) {
                return param->getName() == name;
            });
            if (paramItem != m_paramItems.end()) {
                return *paramItem;
            }
        }
        if (m_type != BlockScopeType::GLOBAL) {
            // 没有找到再去其父作用域中寻找
            return m_symbolTable.getBlockScope(m_parentHandle).findItem(name);
        } else {
            return nullptr;
        }
    }
}

SymbolTableItem* BlockScope::findFunc(const std::string& name) {
    auto item = std::find_if(m_funcs.begin(), m_funcs.end(), [&name](const std::unique_ptr<SymbolTableItem>& func) {
        return func->getName() == name;
    });
    if (item != m_funcs.end()) {
        return item->get();
    } else {
        return nullptr;
    }
}

std::pair<SymbolTableItem*, bool> BlockScope::insertItem(std::unique_ptr<SymbolTableItem>&& item) {
    item->setLevel(m_level);
    // 只在当前scope内插入symbolItem
    auto finded = std::find_if(m_symbols.begin(), m_symbols.end(), [&item](const std::unique_ptr<SymbolTableItem>& symbol) {
        return item->getName() == symbol->getName();
    });
    if (finded != m_symbols.end()) {
        return std::make_pair(finded->get(), false);
    } else {
        m_symbols.push_back(std::move(item));
        return std::make_pair(m_symbols.back().get(), true);
    }
}

std::pair<SymbolTableItem*, bool> BlockScope::insertFunc(std::unique_ptr<SymbolTableItem>&& func) {
    func->setLevel(m_level);
    // 只在当前scope内插入symbolItem
    auto finded = std::find_if(m_funcs.begin(), m_funcs.end(), [&func](const std::unique_ptr<SymbolTableItem>& symbol) {
        return func->getName() == symbol->getName();
    });
    if (finded != m_funcs.end()) {
        return std::make_pair(finded->get(), false);
    } else {
        m_funcs.push_back(std::move(func));
        return std::make_pair(m_funcs.back().get(), true);
    }
}

std::vector<SymbolTableItem*>& BlockScope::getParamItems() const {
    m_paramItems.clear();
    for (auto& item : m_symbols) {
        if (item->isParam()) {
            m_paramItems.push_back(item.get());
        }
    }
    return m_paramItems;
}

void BlockScope::checkFuncScopeReturn(int lineNum) const {
    if (!m_hasReturn && m_type == BlockScopeType::FUNC) {
        Logger::logError(ErrorType::NONVOID_FUNC_MISS_RETURN, lineNum, mp_funcItem->getName());
    }
}

void BlockScope::dumpScope(std::ostream& os) {
    for (auto& func : m_funcs) {
        func->dumpSymbolItem(os);
        os << "\n";
    }
    for (auto& symbol : m_symbols) {
        symbol->dumpSymbolItem(os);
        os << "\n";
    }
}