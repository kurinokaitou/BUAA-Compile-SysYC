#ifndef SYMBOL_TABLE_ITEM_H
#define SYMBOL_TABLE_ITEM_H
#include "symbol/BlockScopeHandle.h"
#include <string>
enum class SymbolTableItemType : unsigned int {
    VAR = 0,
    ARRAY,
    FUNC,
    LITERAL,
};
class SymbolTableItem {
public:
    SymbolTableItem(const std::string& name, BlockScopeHandle handle);
    std::string getName() const { return m_name; }
    bool isParam() const { return m_isParam; }
    void setParam() { m_isParam = true; }

private:
    std::string m_name;
    BlockScopeHandle m_scopeHandle;
    bool m_isParam{false};
};

#endif