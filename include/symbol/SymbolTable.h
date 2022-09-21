#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H
#include "BlockScope.h"
class SymbolTable {
public:
    SymbolTable();
    BlockScope& getBlockScope(BlockScopeHandle handle);
    BlockScope& getCurrentScope();
    void pushScope(BlockScopeType type);
    void popScope();

    template <typename ItemType, typename DataType>
    void insertItem(const std::string& name, DataType data);
    SymbolTableItem* findItem(const std::string& name);

private:
    std::vector<BlockScope> m_blockScopes;
    BlockScopeHandle m_currScopeHandle;
};

template <typename ItemType, typename DataType>
void SymbolTable::insertItem(const std::string& name, DataType data) {
    if (std::is_base_of<SymbolTableItem, ItemType>::value) {
        getCurrentScope().insertItem(std::unique_ptr<ItemType>(new ItemType(name, data)));
    }
}
#endif