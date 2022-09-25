#include <codegen/CodeGenerator.h>

CodeGenerator::CodeGenerator(std::shared_ptr<VNodeBase> astRoot) {
    m_visitor = std::unique_ptr<Visitor>(new Visitor(astRoot, m_table));
}

void CodeGenerator::generate() {
    m_visitor->visit();
}