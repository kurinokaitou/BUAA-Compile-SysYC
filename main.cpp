#include <Compiler.h>

bool Compiler::firstPass(std::filebuf& file) {
    std::filebuf token;
    std::filebuf ast;
    std::filebuf table;
    std::filebuf error;
    std::filebuf ir;
    std::filebuf mips;
    try {
        m_tokenizer = std::unique_ptr<Tokenizer>(new Tokenizer(file));
        m_tokenList = m_tokenizer->tokenize();
        if (s_dumpToken) {
            dumpToken(token);
            token.close();
        }
        m_parser = std::unique_ptr<Parser>(new Parser(m_tokenList));
        m_parser->parse();
        if (s_dumpAST) {
            dumpAST(ast);
            ast.close();
        }

        m_generator = std::unique_ptr<CodeGenerator>(new CodeGenerator(m_parser->getASTRoot()));
        m_generator->generate(s_optLevel, s_dumpMips);
        if (s_dumpIr) {
            dumpIr(ir, s_isTest);
            ir.close();
        }
        if (s_dumpTable) {
            dumpTable(table);
            table.close();
        }
        if (s_dumpError) {
            dumpError(error);
            error.close();
        }
        if (s_dumpMips) {
            dumpMips(mips);
            mips.close();
        }
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
        return false;
    }
    return true;
}

void Compiler::dumpToken(std::filebuf& file) {
    if (!file.open(s_dumpTokenPath, std::ios::out)) {
        throw std::runtime_error("Fail to open the dump token file!");
    }
    std::ostream tos(&file);
    for (auto& token : m_tokenList) {
        tos << token;
    }
}

void Compiler::dumpAST(std::filebuf& file) {
    if (!file.open(s_dumpASTPath, std::ios::out)) {
        throw std::runtime_error("Fail to open the dump ast file!");
    }
    m_parser->traversalAST(file);
}

void Compiler::dumpTable(std::filebuf& file) {
    if (!file.open(s_dumpTablePath, std::ios::out)) {
        throw std::runtime_error("Fail to open the dump table file!");
    }
    m_generator->dumpTable(file);
}

void Compiler::dumpError(std::filebuf& file) {
    if (!file.open(s_dumpErrorPath, std::ios::out)) {
        throw std::runtime_error("Fail to open the dump error file!");
    }
    std::ostream os(&file);
    std::sort(Logger::s_errorDump.begin(), Logger::s_errorDump.end(), [](const ErrorLog& log1, const ErrorLog& log2) -> bool {
        return (log1.lineNum == log2.lineNum) ? (log1.code < log2.code) : (log1.lineNum < log2.lineNum);
    });
    for (auto& log : Logger::s_errorDump) {
        os << log;
    }
}

void Compiler::dumpIr(std::filebuf& file, bool isTest) {
    if (!file.open(s_dumpIrPath, std::ios::out)) {
        throw std::runtime_error("Fail to open the ir file!");
    }
    m_generator->dumpIr(file, isTest);
}

void Compiler::dumpMips(std::filebuf& file) {
    if (!file.open(s_dumpMipsPath, std::ios::out)) {
        throw std::runtime_error("Fail to open the mips file!");
    }
    m_generator->dumpMips(file);
}

int main(int argc, char** argv) {
    Compiler compiler;
    std::filebuf in;
    int ret = 0;

    parseArgs(argc, argv);
    if (!in.open(s_sourcePath, std::ios::in)) {
        std::cerr << "Fail to open the source file!" << std::endl;
        ret = 1;
    }
    ret = compiler.firstPass(in) ? ret : 1;
    return ret;
}
