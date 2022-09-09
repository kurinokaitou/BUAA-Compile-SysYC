#include <exception>
#include <token/Tokenizer.h>

static std::string s_sourcePath = "test/grammar/testfile1.txt";
static std::string s_outputPath = "intermediate/token.txt";

static bool takeArg(char* arg) {
    static const std::set<std::string> x{"-o"};
    return x.count(std::string(arg));
}

static void usage(int status) {
    std::cerr << "usage: SysYC [ -o <path> ] <file>\n";
    exit(status);
}

void parseArgs(int argc, char** argv) {
    // TODO: args with input file or config file
    for (int i = 0; i < argc; i++) {
        if (takeArg(argv[i])) {
            if (!argv[++i])
                usage(1);
        }
    }
    for (int i = 1; i < argc; i++) {
        if (argv[i] == std::string("-o")) {
            s_outputPath = argv[++i];
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
        std::filebuf out;
        if (!in.open(s_sourcePath, std::ios::in)) {
            throw std::runtime_error("Fail to open the source file!");
        }
        if (!out.open(s_outputPath, std::ios::out)) {
            throw std::runtime_error("Fail to open the output file!");
        }
        Tokenizer tokenizer(in);
        auto tokenLists = tokenizer.tokenize();
        // tokenizer debug
        std::ostream os(&out);
        for (auto& token : tokenLists) {
            os << token;
        }
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
}
