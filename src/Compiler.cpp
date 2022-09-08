#include <exception>
#include <token/Tokenizer.h>

static std::string s_sourcePath = "test/grammar/testfile1.txt";
static std::string s_outputPath;

void parseArgs(int argc, char** argv) {
    // TODO: args with input file or config file
}

int main(int argc, char** argv) {
    parseArgs(argc, argv);
    try {
        std::filebuf filebuf;
        if (!filebuf.open(s_sourcePath, std::ios::in)) {
            throw std::runtime_error("Fail to open the source file!");
        }
        Tokenizer tokenizer(filebuf);
        auto tokenLists = tokenizer.tokenize();
        // tokenizer debug
        // for (auto& token : tokenLists) {
        //     std::cout << token;
        // }
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
}
