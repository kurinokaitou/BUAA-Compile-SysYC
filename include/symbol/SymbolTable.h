#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H
#include "BlockScope.h"
class SymbolTable {
public:
    BlockScope& getBlockScope(BlockScopeHandle handle) {
        return m_blockScopes.at(handle.index);
    }

    BlockScope& getCurrentScope() {
        return m_blockScopes.at(m_currScopeHandle.index);
    }

private:
    std::vector<BlockScope> m_blockScopes;
    BlockScopeHandle m_currScopeHandle;
};
#endif