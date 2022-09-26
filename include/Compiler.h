#ifndef COMPILER_H
#define COMPILER_H
#include <exception>
#include <token/Tokenizer.h>
#include <grammar/Parser.h>
#include <symbol/SymbolTable.h>
#include <codegen/CodeGenerator.h>
class Compiler {
public:
    bool firstPass(std::filebuf& file);
    std::vector<Token>& getTokenList();
    std::shared_ptr<VNodeBase> getAstNode();
    void dumpToken(std::filebuf& file);
    void dumpAST(std::filebuf& file);
    void dumpTable(std::filebuf& file);

private:
    std::unique_ptr<Tokenizer> m_tokenizer;
    std::unique_ptr<Parser> m_parser;
    std::unique_ptr<CodeGenerator> m_generator;
    std::vector<Token> m_tokenList;
    std::shared_ptr<VNodeBase> m_astNode;
};
#endif