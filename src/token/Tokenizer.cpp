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

void Tokenizer::skipVacant() {
    while (isspace(m_currChar) || isNewline(m_currChar) || isTab(m_currChar)) {
        if (isNewline(m_currChar)) {
            m_currLine++;
        }
        extractChar();
    }
}

SymbolEnum Tokenizer::readIdent() {
    while (isIdentChar(m_currChar)) {
        m_tokenStr += m_currChar;
        extractChar();
    }
    unextractChar();
    return getReservedWordSymbol(m_tokenStr);
}

SymbolEnum Tokenizer::readInteger() {
    if (m_currChar != '0') {
        while (isdigit(m_currChar)) {
            m_tokenStr += m_currChar;
            extractChar();
        }
        unextractChar();
        m_tokenValue = stringToInt(m_tokenStr);
        return SymbolEnum::INTCON;
    } else {
        m_tokenStr += m_currChar;
        extractChar();
        if (!isdigit(m_currChar)) {
            unextractChar();
            m_tokenValue = stringToInt(m_tokenStr);
            return SymbolEnum::INTCON;
        } else {
            throw std::runtime_error("compile error!");
            // TODO: 前导零错误
            return SymbolEnum::UNKNOWN;
        }
    }
}
SymbolEnum Tokenizer::skipComment() {
    if (m_currChar == '/') { // 单行注释
        while (!isNewline(m_currChar)) {
            extractChar();
        }
        m_currLine++;
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
    }
    return SymbolEnum::COMMENT;
}

SymbolEnum Tokenizer::readPunct() {
    char preChar = m_currChar;
    m_tokenStr += m_currChar;
    auto symbol = getSymbolEnumByPunct(m_tokenStr);
    extractChar();
    m_tokenStr += m_currChar; // 获取第二个
    auto doubleSym = getSymbolEnumByPunct(m_tokenStr);
    if (doubleSym != SymbolEnum::UNKNOWN) {
        symbol = doubleSym;
    } else {
        m_tokenStr.clear();
        m_tokenStr += preChar;
        unextractChar();
    }
    return symbol;
}

SymbolEnum Tokenizer::readString() {
    m_tokenStr += m_currChar;
    extractChar();
    while (m_currChar != '"') {
        m_tokenStr += m_currChar;
        extractChar();
    }
    if (m_currChar == '"') {
        m_tokenStr += m_currChar;
        return SymbolEnum::STRCON;
    } else {
        throw std::runtime_error("compile error!");
        // TODO: 无后引号错误
        return SymbolEnum::UNKNOWN;
    }
}

std::vector<Token>& Tokenizer::tokenize() {
    while (m_istream.get(m_currChar)) {
        m_tokenStr.clear();
        m_symbol = SymbolEnum::UNKNOWN;
        m_tokenValue = 0;
        skipVacant();
        // 处理标识符
        if (isalpha(m_currChar)) {
            m_symbol = readIdent();
        }
        // 处理非零数字
        else if (isdigit(m_currChar)) {
            m_symbol = readInteger();
        }
        // 处理注释和除法
        else if (m_currChar == '/') {
            m_tokenStr += m_currChar;
            extractChar();
            if (m_currChar == '/' || m_currChar == '*') {
                m_symbol = skipComment();
            } else {
                unextractChar();
                m_symbol = SymbolEnum::DIV;
            }
        }
        // 处理分界符号
        else if (isPunct(m_currChar)) {
            m_symbol = readPunct();
        }
        // 处理字面量字符串
        else if (m_currChar == '"') {
            m_symbol = readString();
        }
        if (m_symbol != SymbolEnum::COMMENT) {
            m_currToken = makeToken(m_currLine, m_symbol, m_tokenStr, m_tokenValue);
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
    auto m_symbol = getSymbolEnumByText(copy + "RW");
    return reservedWordSet.count(word) != 0 ? m_symbol : SymbolEnum::IDENT;
}

void Tokenizer::extractChar() {
    m_istream.get(m_currChar);
}

void Tokenizer::unextractChar() {
    m_istream.unget();
}