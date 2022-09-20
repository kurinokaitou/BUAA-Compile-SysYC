#ifndef SYMBOL_TABLE_ITEM_H
#define SYMBOL_TABLE_ITEM_H
#include <string>
enum class SymbolTableItemType : unsigned int {
    VAR = 0,
    ARRAY,
    FUNC,
    LITERAL,
};
class SymbolTableItem {
public:
    std::string getName() const { return m_name; }

private:
    std::string m_name;
};
#endif