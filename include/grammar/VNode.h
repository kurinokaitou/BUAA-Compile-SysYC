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
    using ChidrenIter = std::vector<std::shared_ptr<VNodeBase>>::const_iterator;
    VNodeBase() = default;
    explicit VNodeBase(bool isCorrect) :
        m_isCorrect(isCorrect) {}
    virtual VType getType() const = 0;
    virtual void addChild(std::shared_ptr<VNodeBase>&& child) = 0;
    virtual const std::vector<std::shared_ptr<VNodeBase>>& getChildren(int offset = 0) const = 0;
    virtual ChidrenIter getChildIter(int offset = 0) const = 0;
    virtual size_t getChildrenNum() const = 0;
    virtual bool nextChild(int offset = 1) = 0;
    virtual void dumpToFile(std::ostream& os) = 0;
    virtual VNodeEnum getNodeEnum() const = 0;
    virtual SymbolEnum getSymbol() const = 0;

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
        DBG_ERROR("Leaf node add child error");
    }
    virtual void dumpToFile(std::ostream& os) override {
        for (int i = 1; i < m_level; i++) os << "  ";
        os << getSymbolText(m_symbol) << " " << m_token.literal << "\n";
    }
    virtual const std::vector<std::shared_ptr<VNodeBase>>& getChildren(int offset = 0) const override {
        DBG_ERROR("Try to get children from a leaf node!");
        static const std::vector<std::shared_ptr<VNodeBase>> dummy;
        return dummy;
    };
    virtual ChidrenIter getChildIter(int offset = 0) const override {
        DBG_ERROR("Try to get childIter from a leaf node!");
        return ChidrenIter();
    }
    virtual size_t getChildrenNum() const override {
        DBG_ERROR("Try to get children number from a leaf node!");
        return 0;
    }
    virtual bool nextChild(int offset = 1) override {
        DBG_ERROR("Try to iterate children nodes of a leaf node!");
        return false;
    }
    virtual VNodeEnum getNodeEnum() const override {
        DBG_ERROR("Try to get node enum of a leaf node!");
        return VNodeEnum::UNKNOWNN;
    }
    virtual SymbolEnum getSymbol() const override { return m_symbol; }
    const Token& getToken() const { return m_token; }

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
        m_currentChild = m_childrenNodes.begin();
    }
    virtual void dumpToFile(std::ostream& os) override {
        for (int i = 1; i < m_level; i++) os << "  ";
        os << "<" << getVNodeEnumText(m_nodeEnum) << ">\n";
    }
    virtual const std::vector<std::shared_ptr<VNodeBase>>& getChildren(int offset = 0) const override { return m_childrenNodes; };
    virtual ChidrenIter getChildIter(int offset = 0) const override { return m_currentChild + offset; }
    virtual size_t getChildrenNum() const override {
        return m_childrenNodes.size();
    }
    virtual bool nextChild(int offset = 1) override {
        m_currentChild += offset;
        if (m_currentChild < m_childrenNodes.end()) {
            return true;
        } else {
            m_currentChild -= offset;
            DBG_LOG("Iterator out of range!");
            return false;
        }
    }
    virtual VNodeEnum getNodeEnum() const override { return m_nodeEnum; }
    virtual SymbolEnum getSymbol() const override {
        DBG_ERROR("Try to get symbol enum of a branch node!");
        return SymbolEnum::UNKNOWN;
    }

private:
    VNodeEnum m_nodeEnum{VNodeEnum::COMPUNIT};
    std::vector<std::shared_ptr<VNodeBase>> m_childrenNodes;
    ChidrenIter m_currentChild;
};
#endif