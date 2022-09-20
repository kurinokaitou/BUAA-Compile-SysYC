#ifndef BLOCK_HANLDE_H
#define BLOCK_HANLDE_H
#include <limits>
// 用于
class BlockScopeHandle {
    friend class SymbolTable;
    friend class BlockScope;
    using Index = std::size_t;
    static const Index UNINITIALIZED = std::numeric_limits<Index>::max();
    Index index{UNINITIALIZED};

public:
    BlockScopeHandle() = default;
    explicit BlockScopeHandle(Index idx) :
        index(idx) {}
    BlockScopeHandle(const BlockScopeHandle& handle) = default;
    BlockScopeHandle& operator=(const BlockScopeHandle& handle) = default;
    bool isInitialized() const { return index != UNINITIALIZED; }
    void clear() { index = UNINITIALIZED; }
    operator bool() const { return isInitialized(); }
    bool operator<(const BlockScopeHandle& rhs) {
        return (index < rhs.index);
    }
    bool operator>(const BlockScopeHandle& rhs) {
        return (index > rhs.index);
    }
    bool operator==(const BlockScopeHandle& rhs) {
        return (index == rhs.index);
    }
    bool operator!=(const BlockScopeHandle& rhs) {
        return (index != rhs.index);
    }
};

#endif