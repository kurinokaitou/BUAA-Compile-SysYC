#include <symbol/BlockScope.h>
#include <symbol/SymbolTable.h>
BlockScope::BlockScope(SymbolTable& table, BlockScopeType type, int level, BlockScopeHandle parent) :
    m_symbolTable(table), m_type(type), m_level(level), m_parentHandle(parent) {
}

void BlockScope::addChildScope(BlockScopeHandle handle) {
    m_childrenHandle.push_back(handle);
}

SymbolTableItem* BlockScope::findItem(const std::string& name) {
    auto item = m_symbols.find(name);
    if (item != m_symbols.end()) {
        return item->second.get();
    } else {
        if (m_type != BlockScopeType::GLOBAL) {
            return m_symbolTable.getBlockScope(m_parentHandle).findItem(name);
        } else {
            return nullptr;
        }
    }
}

void BlockScope::insertItem(std::unique_ptr<SymbolTableItem>&& item) {
    item->setLevel(m_level);
    m_symbols.insert(std::make_pair(item->getName(), std::move(item)));
}

std::vector<SymbolTableItem*>& BlockScope::getParamItems() const {
    m_paramItems.clear();
    for (auto& item : m_symbols) {
        if (item.second->isParam()) {
            m_paramItems.push_back(item.second.get());
        }
    }
    return m_paramItems;
}