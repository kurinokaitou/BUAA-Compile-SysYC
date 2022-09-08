#include <token/Tokenizer.h>
#include <Utils.h>
#include <set>

Tokenizer::Tokenizer(std::filebuf& fileBuf) :
    m_istream(&fileBuf) {
}

Tokenizer::TokenIter Tokenizer::makeToken(int lineNum, const SymbolEnum& symbolNum, const std::string& literal, int value) {
    m_tokenList.emplace_back(lineNum, symbolNum, literal, value);
    return m_tokenList.end() - 1;
}

std::vector<Token>& Tokenizer::tokenize() {
    SymbolEnum symbol{SymbolEnum::UNKNOWN};
    std::string tokenStr;
    int tokenValue = 0;
    while (m_istream.get(m_currChar)) {
        tokenStr.clear();
        symbol = SymbolEnum::UNKNOWN;
        tokenValue = 0;
        while (isspace(m_currChar) || isNewline(m_currChar) || isTab(m_currChar)) {
            if (isNewline(m_currChar)) {
                m_currLine++;
            }
            extractChar();
        }
        // 进入标识符
        if (isalpha(m_currChar)) {
            while (isIdentChar(m_currChar)) {
                tokenStr += m_currChar;
                extractChar();
            }
            m_istream.unget();
            symbol = getReservedWordSymbol(tokenStr);
        }
        // 进入非零数字
        else if (isdigit(m_currChar)) {
            if (m_currChar != '0') {
                while (isdigit(m_currChar)) {
                    tokenStr += m_currChar;
                    extractChar();
                }
                m_istream.unget();
                symbol = SymbolEnum::INTCON;
            } else {
                tokenStr += m_currChar;
                extractChar();
                if (!isdigit(m_currChar)) {
                    m_istream.unget();
                    symbol = SymbolEnum::INTCON;
                } else {
                    // TODO: 前导零错误
                }
            }
        }
        // 进入注释
        else if (m_currChar == '/') {
            tokenStr += m_currChar;
            extractChar();
            if (m_currChar == '/') { // 单行注释
                while (!isNewline(m_currChar)) {
                    extractChar();
                }
                m_currLine++;
                symbol = SymbolEnum::COMMENT;
            } else if (m_currChar == '*') { // 多行注释
                extractChar();
                while (true) {
                    while (m_currChar != '*') {
                        if (isNewline(m_currChar)) {
                            m_currLine++;
                        }
                        extractChar();
                    }
                    extractChar();
                    if (m_currChar == '/') {
                        break;
                    } else {
                        continue;
                    }
                }
                symbol = SymbolEnum::COMMENT;
            } else { // 除法
                unextractChar();
                symbol = SymbolEnum::DIV;
            }
        }

        if (symbol != SymbolEnum::COMMENT) {
            m_currToken = makeToken(m_currLine, symbol, tokenStr, tokenValue);
        }
    }

    return m_tokenList;
}

SymbolEnum Tokenizer::getReservedWordSymbol(std::string& word) const {
    static const std::set<std::string> reservedWordSet{
        "const",
        "main",
        "int",
        "void",
        "return",
        "if",
        "else",
        "break",
        "continue",
        "while",
        "getint",
        "printf",
    };
    std::string copy(word);
    toUpper(copy);
    auto symbol = getSymbolEnumByText(copy + "RW");
    return reservedWordSet.count(word) != 0 ? symbol : SymbolEnum::IDENT;
}

void Tokenizer::extractChar() {
    m_istream.get(m_currChar);
}

void Tokenizer::unextractChar() {
    m_istream.unget();
}