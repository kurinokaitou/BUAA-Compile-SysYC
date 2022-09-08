#include <iostream>
#include <exception>
#include <token/Tokenizer.h>

static std::string s_sourcePath;
static std::string s_outputPath;

void parseArgs(int argc, char** argv) {
    // TODO: args with input file or config file
}

int main(int argc, char** argv) {
    parseArgs(argc, argv);
    try {
        Tokenizer tokenizer(s_sourcePath);
        auto tokenLists = tokenizer.tokenize();
        // tokenizer debug
        for (auto& token : tokenLists) {
            std::cout << token;
        }
    } catch (std::exception e) {
        std::cerr << e.what() << std::endl;
    }
}
