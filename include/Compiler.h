#ifndef COMPILER_H
#define COMPILER_H
#include <exception>
#include <token/Tokenizer.h>
#include <grammar/Parser.h>
#include <symbol/SymbolTable.h>
#include <codegen/CodeGenerator.h>

static std::string s_sourcePath = "testfile.txt";
static std::string s_dumpTokenPath = "token.txt";
static std::string s_dumpASTPath = "ast.txt";
static std::string s_dumpTablePath = "table.txt";
static std::string s_dumpErrorPath = "error.txt";
static bool s_dumpToken = false;
static bool s_dumpAST = true;
static bool s_dumpTable = true;
static bool s_dumpError = true;

static bool takeArg(char* arg) {
    static const std::set<std::string> x{"-o", "--dump-token", "--dump-ast", "--dump-table"};
    return x.count(std::string(arg));
}

static void usage(int status) {
    std::cerr << "usage: sysyc [--dump-token <path>] [--dump-ast <path>] [ -o <path> ] <path>\n";
    std::cerr << "usage: sysyc [--test]\n";
    exit(status);
}

static void parseArgs(int argc, char** argv) {
    // TODO: 参数加入配置文件
    for (int i = 0; i < argc; i++) {
        if (takeArg(argv[i])) {
            if (!argv[++i])
                usage(1);
        }
    }
    for (int i = 1; i < argc; i++) {
        if (argv[i] == std::string("--dump-token")) {
            s_dumpTokenPath = argv[++i];
            s_dumpToken = true;
            continue;
        }
        if (argv[i] == std::string("--dump-ast")) {
            s_dumpASTPath = argv[++i];
            s_dumpAST = true;
            continue;
        }
        if (argv[i] == std::string("--dump-table")) {
            s_dumpTablePath = argv[++i];
            s_dumpTable = true;
            continue;
        }
        if (argv[i] == std::string("--dump-error")) {
            s_dumpErrorPath = argv[++i];
            s_dumpError = true;
            continue;
        }
        if (argv[i] == std::string("--test")) {
            s_dumpToken = true;
            s_dumpAST = true;
            s_dumpTable = true;
            s_dumpError = true;
            break;
        }
        s_sourcePath = std::string(argv[i]);
    }
}

class Compiler {
public:
    bool firstPass(std::filebuf& file);
    std::vector<Token>& getTokenList();
    std::shared_ptr<VNodeBase> getAstNode();
    void dumpToken(std::filebuf& file);
    void dumpAST(std::filebuf& file);
    void dumpTable(std::filebuf& file);
    void dumpError(std::filebuf& file);

private:
    std::unique_ptr<Tokenizer> m_tokenizer;
    std::unique_ptr<Parser> m_parser;
    std::unique_ptr<CodeGenerator> m_generator;
    std::vector<Token> m_tokenList;
    std::shared_ptr<VNodeBase> m_astNode;
};
#endif