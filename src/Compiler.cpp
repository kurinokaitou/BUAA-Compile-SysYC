#include <exception>
#include <token/Tokenizer.h>

static std::string s_sourcePath = "test/grammar/testfile1.txt";
static std::string s_outputPath = "intermediate/token.txt";

void parseArgs(int argc, char** argv) {
    // TODO: args with input file or config file
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
