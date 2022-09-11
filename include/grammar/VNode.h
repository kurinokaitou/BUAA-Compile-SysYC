#ifndef AST_NODE_H
#define AST_NODE_H
#include <token/Tokenizer.h>
#include "VNodeEnum.h"
#include <memory>
#include <vector>
#include <utility>

class VNodeBase {
public:
    VNodeBase() = default;
    explicit VNodeBase(bool isCorrect);
    virtual VType getType() const = 0;

public:
    int getLevel() const { return m_level; }
    std::weak_ptr<VNodeBase> getParent() { return m_parent; }
    void setParent(const std::shared_ptr<VNodeBase> parent);
    void setCorrect() { m_isCorrect = true; }

protected:
    std::weak_ptr<VNodeBase> m_parent;
    int m_level{-1};
    bool m_isCorrect{false};
};

// TODO: finish ast branch and leaf
class VNodeLeaf : public VNodeBase {
public:
    explicit VNodeLeaf(SymbolEnum symbol, Token token, bool isCorrect = true);
    virtual VType getType() const override { return VType::VT; }

private:
    SymbolEnum m_symbol{SymbolEnum::UNKNOWN};
    Token m_token;
};

class VNodeBranch : public VNodeBase {
public:
    explicit VNodeBranch(VNodeType nodeType);
    virtual VType getType() const override { return VType::VN; }

private:
    VNodeType m_nodeType;
    std::vector<std::shared_ptr<VNodeBase>> m_childrenNodes;
};
#endif