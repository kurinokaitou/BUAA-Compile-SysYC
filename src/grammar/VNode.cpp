#include <grammar/VNode.h>
VNodeBase::VNodeBase(bool isCorrect) :
    m_isCorrect(isCorrect) {}

void VNodeBase::setParent(const std::shared_ptr<VNodeBase> parent) {
    m_parent = std::weak_ptr<VNodeBase>(parent);
    m_level = parent->getLevel() + 1;
}

VNodeLeaf::VNodeLeaf(SymbolEnum symbol, Token token, bool isCorrect) :
    VNodeBase(isCorrect),
    m_symbol(symbol), m_token(token) {
}