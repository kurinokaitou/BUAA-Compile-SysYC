#ifndef CODE_GENERATOR_H
#define CODE_GENERATOR_H
#include "Vistitor.h"
class CodeGenerator {
public:
    CodeGenerator(std::shared_ptr<VNodeBase> astRoot);
    void generate();

private:
    std::unique_ptr<Visitor> m_visitor;
    SymbolTable m_table;
};
#endif
