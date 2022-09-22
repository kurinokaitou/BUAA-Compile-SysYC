#include <symbol/SymbolTable.h>
SymbolTable::SymbolTable() :
    m_currScopeHandle(BlockScopeHandle(0)) {
    m_blockScopes.emplace_back(*this, BlockScopeType::GLOBAL, 0, BlockScopeHandle());
}

BlockScope& SymbolTable::getBlockScope(BlockScopeHandle handle) {
    return m_blockScopes.at(handle.index);
}

BlockScope& SymbolTable::getCurrentScope() {
    return m_blockScopes.at(m_currScopeHandle.index);
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
    m_blockScopes.pop_back();
}

SymbolTableItem* SymbolTable::findItem(const std::string& name) {
    return getCurrentScope().findItem(name);
}

void SymbolTable::clearSymbolTable() {
    m_blockScopes.clear();
}