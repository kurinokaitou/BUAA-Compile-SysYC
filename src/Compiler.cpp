#include <exception>
#include <token/Tokenizer.h>
#include <grammar/Parser.h>

static std::string s_sourcePath = "test/grammar/test.txt";
static std::string s_dumpTokenPath = "intermediate/token.txt";
static std::string s_dumpASTPath = "intermediate/ast.txt";
static bool s_dumpToken = false;
static bool s_dumpAST = false;

static bool takeArg(char* arg) {
    static const std::set<std::string> x{"-o", "--dump-token", "--dump-ast"};
    return x.count(std::string(arg));
}

static void usage(int status) {
    std::cerr << "usage: sysyc [--dump-token <path>] [--dump-ast <path>] [ -o <path> ] <path>\n";
    std::cerr << "usage: sysyc [--test]\n";
    exit(status);
}

void parseArgs(int argc, char** argv) {
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
        if (argv[i] == std::string("--test")) {
            s_dumpToken = true;
            s_dumpAST = true;
            break;
        }
        s_sourcePath = std::string(argv[i]);
    }
}

int main(int argc, char** argv) {
    parseArgs(argc, argv);
    bool error = false;
    std::vector<Token> tokenList;
    std::shared_ptr<VNodeBase> astNode;
    std::filebuf token;
    std::filebuf ast;
    try {
        std::filebuf in;

        if (!in.open(s_sourcePath, std::ios::in)) {
            throw std::runtime_error("Fail to open the source file!");
        }

        Tokenizer tokenizer(in);
        tokenList = tokenizer.tokenize();
        Parser grammarParser(tokenList);
        astNode = grammarParser.parse();
    } catch (std::exception& e) {
        error = true;
        std::cerr << e.what() << std::endl;
    }
    // dump token
    if (s_dumpToken) {
        if (!token.open(s_dumpTokenPath, std::ios::out)) {
            throw std::runtime_error("Fail to open the dump token file!");
        }
        std::ostream tos(&token);
        for (auto& token : tokenList) {
            tos << token;
        }
    }
    // dump ast
    if (s_dumpAST) {
        if (!ast.open(s_dumpASTPath, std::ios::out)) {
            throw std::runtime_error("Fail to open the dump ast file!");
        }
        if (!error) {
            std::ostream aos(&ast);
            astNode->dumpToFile(aos);
            return 0;
        } else {
            return 1;
        }
    }
}
