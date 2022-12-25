#include <codegen/CodeGenerator.h>

CodeGenerator::CodeGenerator(std::shared_ptr<VNodeBase> astRoot) {
    m_visitor = std::unique_ptr<Visitor>(new Visitor(std::move(astRoot), m_table, m_irCtx));
}

void CodeGenerator::generate(int optLevel, bool genMips) {
    m_visitor->visit();
    m_irCtx.module.calPredSucc();
    m_irCtx.module.addImplicitReturn();
    if (optLevel) {
        m_irCtx.module.optimizeIrCode(optLevel);
    }
    // Logger::logInfo("optLevel=" + std::to_string(optLevel) + "\n");
    if (genMips) {
        m_mipsCtx.convertMipsCode(m_irCtx.module);
        if (optLevel) {
            m_mipsCtx.optimizeMipsCode(optLevel);
        }
    }
}

void CodeGenerator::dumpTable(std::filebuf& file) {
    std::ostream os(&file);
    m_table.dumpTable(os);
}

void CodeGenerator::dumpIr(std::filebuf& file, bool isTest) {
    std::ostream os(&file);
    m_irCtx.module.toCode(os, isTest);
}

void CodeGenerator::dumpMips(std::filebuf& file) {
    std::ostream os(&file);
    m_mipsCtx.m_module.toCode(os);
}