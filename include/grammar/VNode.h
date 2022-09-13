#ifndef AST_NODE_H
#define AST_NODE_H
#include <token/Tokenizer.h>
#include <Log.h>
#include "VNodeEnum.h"
#include <memory>
#include <vector>
#include <utility>

class VNodeBase {
public:
    VNodeBase() = default;
    explicit VNodeBase(bool isCorrect) :
        m_isCorrect(isCorrect) {}
    virtual VType getType() const = 0;
    virtual void addChild(std::shared_ptr<VNodeBase>&& child) = 0;
    virtual void dumpToFile(std::ostream& os) = 0;

public:
    int getLevel() const { return m_level; }
    void setLevel(int level) { m_level = level; }
    std::weak_ptr<VNodeBase> getParent() { return m_parent; }
    void setParent(const std::shared_ptr<VNodeBase> parent) {
        m_parent = std::weak_ptr<VNodeBase>(parent);
    }
    void setCorrect(bool correct) { m_isCorrect = correct; }
    bool isCorrect() const { return m_isCorrect; }

protected:
    std::weak_ptr<VNodeBase> m_parent;
    int m_level{-1};
    bool m_isCorrect{false};
};

class VNodeLeaf : public VNodeBase {
public:
    explicit VNodeLeaf(SymbolEnum symbol, Token token, bool isCorrect = true) :
        VNodeBase(isCorrect),
        m_symbol(symbol), m_token(token) {}
    virtual VType getType() const override { return VType::VT; }
    virtual void addChild(std::shared_ptr<VNodeBase>&& child) override {
        PARSER_LOG_ERROR("Leaf node add child error");
    }
    virtual void dumpToFile(std::ostream& os) override {
        os << getSymbolText(m_symbol) << " " << m_token.literal << "\n";
    }

private:
    SymbolEnum m_symbol{SymbolEnum::UNKNOWN};
    Token m_token;
};

class VNodeBranch : public VNodeBase, public std::enable_shared_from_this<VNodeBranch> {
public:
    explicit VNodeBranch(VNodeEnum nodeEnum, bool isCorrect = true) :
        VNodeBase(isCorrect),
        m_nodeEnum(nodeEnum) {}
    virtual VType getType() const override { return VType::VN; }
    virtual void addChild(std::shared_ptr<VNodeBase>&& child) override {
        child->setParent(shared_from_this());
        if (!child->isCorrect()) {
            setCorrect(false);
        }
        m_childrenNodes.push_back(std::move(child));
    }
    virtual void dumpToFile(std::ostream& os) override {
        os << "<" << getVNodeEnumText(m_nodeEnum) << ">\n";
    }

public:
    VNodeEnum getNodeEnum() const { return m_nodeEnum; }
    std::vector<std::shared_ptr<VNodeBase>>& getChildren() { return m_childrenNodes; }

private:
    VNodeEnum m_nodeEnum{VNodeEnum::COMPUNIT};
    std::vector<std::shared_ptr<VNodeBase>> m_childrenNodes;
};
#endif