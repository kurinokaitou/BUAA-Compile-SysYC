#ifndef BLOCK_SCOPE_H
#define BLOCK_SCOPE_H
#include "BlockScopeHandle.h"
#include "SymbolTableItem.h"
#include <map>
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
    explicit BlockScope(SymbolTable& table, BlockScopeType type, int level, BlockScopeHandle parent);
    SymbolTableItem* findItem(const std::string& name);
    void insertItem(std::unique_ptr<SymbolTableItem>&& item);
    std::vector<SymbolTableItem*>& getParamItems() const;
    int getLevel() const { return m_level; }
    BlockScopeHandle getParentHandle() const { return m_parentHandle; }
    void addChildScope(BlockScopeHandle handle);
    std::size_t countSymbol() { return m_symbols.size(); }

private:
    SymbolTable& m_symbolTable;
    BlockScopeHandle m_parentHandle;
    std::vector<BlockScopeHandle> m_childrenHandle;
    int m_level{0};
    BlockScopeType m_type;
    std::map<std::string, std::unique_ptr<SymbolTableItem>> m_symbols;
    mutable std::vector<SymbolTableItem*> m_paramItems;
};

#endif