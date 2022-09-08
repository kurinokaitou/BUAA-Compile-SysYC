#include <token/Tokenizer.h>
#include <Utils.h>

Tokenizer::Tokenizer(std::filebuf& fileBuf) :
    m_istream(&fileBuf) {
}

Tokenizer::TokenIter Tokenizer::makeToken(int lineNum, const SymbolEnum& symbolNum, const std::string& literal, int value) {
    m_tokenList.emplace_back(lineNum, symbolNum, literal, value);
    return m_tokenList.end() - 1;
}

std::vector<Token>& Tokenizer::tokenize() {
    while (m_istream.get(m_currChar)) {
        m_tokenStr.clear();
        while (isspace(m_currChar) || isNewline(m_currChar) || isTab(m_currChar)) {
            if (isNewline(m_currChar)) {
                m_currLine++;
            }
            m_istream.get(m_currChar);
        }
    }

    return m_tokenList;
}
