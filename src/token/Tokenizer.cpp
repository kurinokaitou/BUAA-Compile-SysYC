#include <token/Tokenizer.h>

Tokenizer::Tokenizer(std::string& sourcePath) {
    std::filebuf filebuf;
    if (!filebuf.open(sourcePath, std::ios::in)) {
        throw std::runtime_error("Fail to open the source file");
    }
    m_istream = std::make_unique<std::istream>(&filebuf);
}

Tokenizer::TokenIter Tokenizer::makeToken(int lineNum, const SymbolEnum& symbolNum, const std::string& literal, int value) {
    m_tokensList.emplace_back();
    return m_tokensList.end();
}

std::vector<Token>& Tokenizer::tokenize() {
    // TODO: deal with tokenize
    return m_tokensList;
}
