#include <codegen/CodeGenerator.h>

CodeGenerator::CodeGenerator(std::shared_ptr<VNodeBase> astRoot) {
    m_visitor = std::unique_ptr<Visitor>(new Visitor(std::move(astRoot), m_table));
}

void CodeGenerator::generate() {
    m_visitor->visit();
}

void CodeGenerator::dumpTable(std::filebuf& file) {
    std::ostream os(&file);
    m_table.dumpTable(os);
}