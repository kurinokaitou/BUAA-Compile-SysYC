#ifndef BLOCK_SCOPE_H
#define BLOCK_SCOPE_H
#include "BlockScopeHandle.h"
#include "SymbolTableItem.h"
#include <vector>
#include <memory>
enum class BlockScopeType : unsigned int {
    NORMAL = 0,
    FUNC,
    GLOBAL,
    BRANCH,
    LOOP,
};

class SymbolTable;
class BlockScope {
public:
    explicit BlockScope(SymbolTable& table, BlockScopeType type) :
        m_symbolTable(table), m_type(type) {
    }
    SymbolTableItem* findItem(const std::string& name);
    void insertItem(std::unique_ptr<SymbolTableItem>&& item);

private:
    SymbolTable& m_symbolTable;
    BlockScopeHandle m_parentHandle;
    std::vector<BlockScopeHandle> m_childrenHandle;
    int m_level{0};
    BlockScopeType m_type;
    std::vector<std::unique_ptr<SymbolTableItem>> m_symbols;

private:
    BlockScope(const BlockScope&) = delete;
    BlockScope& operator=(const BlockScope&) = delete;
};
#endif