#include <exception>
#include <token/Tokenizer.h>
#include <grammar/Parser.h>

static std::string s_sourcePath = "test/grammar/test.txt";
static std::string s_dumpTokenPath = "intermediate/token.txt";
static std::string s_dumpASTPath = "intermediate/ast.txt";

static bool takeArg(char* arg) {
    static const std::set<std::string> x{"-o"};
    return x.count(std::string(arg));
}

static void usage(int status) {
    std::cerr << "usage: SysYC [ -o <path> ] <file>\n";
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
            continue;
        }
        if (argv[i] == std::string("--dump-ast")) {
            s_dumpASTPath = argv[++i];
            continue;
        }
        if (argv[i] == std::string("-c")) {
            break;
        }
        s_sourcePath = std::string(argv[i]);
    }
}

int main(int argc, char** argv) {
    parseArgs(argc, argv);
    try {
        std::filebuf in;
        std::filebuf token;
        std::filebuf ast;
        if (!in.open(s_sourcePath, std::ios::in)) {
            throw std::runtime_error("Fail to open the source file!");
        }
        if (!token.open(s_dumpTokenPath, std::ios::out)) {
            throw std::runtime_error("Fail to open the dump token file!");
        }
        if (!ast.open(s_dumpASTPath, std::ios::out)) {
            throw std::runtime_error("Fail to open the dump ast file!");
        }
        Tokenizer tokenizer(in);
        auto tokenLists = tokenizer.tokenize();
        // dump token
        std::ostream tos(&token);
        for (auto& token : tokenLists) {
            tos << token;
        }

        Parser grammarParser(tokenLists);
        auto astNode = grammarParser.parse();
        // dump ast
        std::ostream aos(&ast);
        astNode->dumpToFile(aos);
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
        exit(1);
    }
    return 0;
}
