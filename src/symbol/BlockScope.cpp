#include <symbol/BlockScope.h>
#include <algorithm>

SymbolTableItem* BlockScope::findItem(const std::string& name) {
    auto iter = std::find_if(m_symbols.begin(), m_symbols.end(), [&](std::unique_ptr<SymbolTableItem> item) {
        if (item->getName() == name) {
            return true;
        } else {
            return false;
        }
    });

    return iter->get();
}

void BlockScope::insertItem(std::unique_ptr<SymbolTableItem>&& item) {
}