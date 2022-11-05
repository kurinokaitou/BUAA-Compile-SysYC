#include <symbol/SymbolTable.h>
#include <Log.h>
SymbolTable::SymbolTable() :
    m_currScopeHandle(BlockScopeHandle(0)) {
    m_blockScopes.emplace_back(*this, BlockScopeType::GLOBAL, 0, BlockScopeHandle());
}

std::pair<FuncItem*, bool> SymbolTable::insertFunc(const std::string& name, typename FuncItem::Data data) {
    if (std::is_base_of<SymbolTableItem, FuncItem>::value) {
        return getGlobalScope().insertFunc(std::unique_ptr<FuncItem>(new FuncItem(name, data)));
    } else {
        return std::make_pair(nullptr, false);
    }
}

void SymbolTable::initSymbolTable() {
    m_currScopeHandle = BlockScopeHandle(0);
    m_blockScopes.emplace_back(*this, BlockScopeType::GLOBAL, 0, BlockScopeHandle());
}

BlockScope& SymbolTable::getBlockScope(BlockScopeHandle handle) {
    if (handle.index >= m_blockScopes.size()) {
        DBG_ERROR("handle index out of range:" + std::to_string(handle.index) + "\n");
    }
    return m_blockScopes.at(handle.index);
}

BlockScope& SymbolTable::getCurrentScope() {
    if (m_currScopeHandle.index >= m_blockScopes.size()) {
        DBG_ERROR("current handle index out of range:" + std::to_string(m_currScopeHandle.index) + "\n");
    }
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
    auto parentHandle = m_currScopeHandle;
    m_currScopeHandle = BlockScopeHandle(m_blockScopes.size());
    currentScope.addChildScope(m_currScopeHandle);
    m_blockScopes.push_back(BlockScope(*this, type, currentScope.getLevel() + 1, parentHandle));
}

void SymbolTable::popScope() {
    m_currScopeHandle = getCurrentScope().getParentHandle();
    // m_blockScopes.pop_back();
}

SymbolTableItem* SymbolTable::findItem(const std::string& name) {
    return getCurrentScope().findItem(name);
}

FuncItem* SymbolTable::findFunc(const std::string& name) {
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