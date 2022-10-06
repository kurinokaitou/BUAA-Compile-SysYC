#include <symbol/SymbolTable.h>
SymbolTable::SymbolTable() :
    m_currScopeHandle(BlockScopeHandle(0)) {
    m_blockScopes.emplace_back(*this, BlockScopeType::GLOBAL, 0, BlockScopeHandle());
}

template <>
std::pair<FuncItem*, bool> SymbolTable::insertItem<FuncItem>(const std::string& name, typename FuncItem::Data data) {
    if (std::is_base_of<SymbolTableItem, FuncItem>::value) {
        auto pair = getGlobalScope().insertFunc(std::unique_ptr<FuncItem>(new FuncItem(name, data)));
        return std::make_pair(dynamic_cast<FuncItem*>(pair.first), pair.second);
    } else {
        return std::make_pair(nullptr, false);
    }
}

void SymbolTable::initSymbolTable() {
    m_currScopeHandle = BlockScopeHandle(0);
    m_blockScopes.emplace_back(*this, BlockScopeType::GLOBAL, 0, BlockScopeHandle());
}

BlockScope& SymbolTable::getBlockScope(BlockScopeHandle handle) {
    return m_blockScopes.at(handle.index);
}

BlockScope& SymbolTable::getCurrentScope() {
    return m_blockScopes.at(m_currScopeHandle.index);
}

BlockScope& SymbolTable::getGlobalScope() {
    return m_blockScopes.at(0);
}

BlockScopeHandle SymbolTable::getCurrentScopeHandle() {
    return m_currScopeHandle;
}

void SymbolTable::pushScope(BlockScopeType type) {
    auto& currentScope = getCurrentScope();
    m_blockScopes.emplace_back(*this, type, currentScope.getLevel() + 1, m_currScopeHandle);
    BlockScopeHandle child(m_blockScopes.size() - 1);
    currentScope.addChildScope(child);
    m_currScopeHandle = child;
}

void SymbolTable::popScope() {
    m_currScopeHandle = getCurrentScope().getParentHandle();
    // m_blockScopes.pop_back();
}

SymbolTableItem* SymbolTable::findItem(const std::string& name) {
    return getCurrentScope().findItem(name);
}

SymbolTableItem* SymbolTable::findFunc(const std::string& name) {
    return getGlobalScope().findFunc(name);
}

void SymbolTable::clearSymbolTable() {
    m_blockScopes.clear();
}

void SymbolTable::dumpTable(std::ostream& os) {
    for (auto& scope : m_blockScopes) {
        scope.dumpScope(os);
    }
}