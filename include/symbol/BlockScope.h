#ifndef BLOCK_SCOPE_H
#define BLOCK_SCOPE_H
#include "BlockScopeHandle.h"
#include "SymbolTableItem.h"
#include <vector>
#include <memory>
#include <algorithm>
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
    FuncItem* findFunc(const std::string& name);
    std::pair<SymbolTableItem*, bool> insertItem(std::unique_ptr<SymbolTableItem>&& item);
    std::pair<FuncItem*, bool> insertFunc(std::unique_ptr<FuncItem>&& func);
    std::vector<SymbolTableItem*>& getParamItems() const;
    int getLevel() const { return m_level; }
    BlockScopeType getType() const { return m_type; }
    BlockScopeHandle getParentHandle() const { return m_parentHandle; }
    void addChildScope(BlockScopeHandle handle);
    void setFuncItem(FuncItem* func) { mp_funcItem = func; }
    FuncItem* getFuncItem() const { return mp_funcItem; }
    std::size_t countSymbol() { return m_symbols.size(); }
    void markHasReturn() { m_hasReturn = true; }
    void checkFuncScopeReturn(int lineNum) const;
    void dumpScope(std::ostream& os);

private:
    SymbolTable& m_symbolTable;
    BlockScopeHandle m_parentHandle;
    std::vector<BlockScopeHandle> m_childrenHandle;
    int m_level{0};
    BlockScopeType m_type;
    std::vector<std::unique_ptr<SymbolTableItem>> m_symbols;
    std::vector<std::unique_ptr<FuncItem>> m_funcs;
    mutable std::vector<SymbolTableItem*> m_paramItems;
    FuncItem* mp_funcItem{nullptr};
    bool m_hasReturn{false};
};

#endif