#include <token/Tokenizer.h>
#include <Utils.h>
#include <Log.h>

Tokenizer::Tokenizer(std::filebuf& fileBuf) :
    m_istream(&fileBuf) {
}

Tokenizer::TokenIter Tokenizer::makeToken(int lineNum, const SymbolEnum& symbolNum, const std::string& literal, int value) {
    m_tokenList.emplace_back(lineNum, symbolNum, literal, value);
    return m_tokenList.end() - 1;
}

bool Tokenizer::skipVacant() {
    while ((isspace(m_currChar) || isNewline(m_currChar) || isTab(m_currChar)) && !m_istream.eof()) {
        if (isNewline(m_currChar)) {
            m_currLine++;
        }
        extractChar();
    }
    return m_istream.eof();
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
    SymbolEnum ret = SymbolEnum::UNKNOWN;
    if (m_currChar != '0') {
        while (isdigit(m_currChar)) {
            m_tokenStr += m_currChar;
            extractChar();
        }
        unextractChar();
        m_tokenValue = std::stoi(m_tokenStr);
        ret = SymbolEnum::INTCON;
    } else {
        m_tokenStr += m_currChar;
        extractChar();
        if (!isdigit(m_currChar)) {
            unextractChar();
            m_tokenValue = std::stoi(m_tokenStr);
            ret = SymbolEnum::INTCON;
        } else {
            // TODO: 前导零错误
            ret = SymbolEnum::UNKNOWN;
        }
    }
    return ret;
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
    while (true) {
        m_tokenStr += m_currChar;
        checkFormatChar();
        extractChar();
        if (m_currChar == '"') break;
    }
    if (m_currChar == '"') {
        m_tokenStr += m_currChar;
        return SymbolEnum::STRCON;
    } else {
        // TODO: 无后引号错误
        return SymbolEnum::UNKNOWN;
    }
}

void Tokenizer::checkFormatChar() {
    if (!(m_currChar == ' ' || m_currChar == '!' || (m_currChar >= '(' && m_currChar <= '~'))) {
        if (m_currChar == '%') {
            extractChar();
            if (m_currChar != 'd') {
                Logger::logError(ErrorType::ILLEGAL_SYMBOL, m_currLine, std::string("%").append(1, m_currChar));
            }
            m_tokenStr += m_currChar;
            //unextractChar();
        } else if (m_currChar == '"') {
            m_tokenStr += m_currChar;
            unextractChar();
        } else {
            Logger::logError(ErrorType::ILLEGAL_SYMBOL, m_currLine, std::string(1, m_currChar));
        }

    } else {
        if (m_currChar == '\\') {
            extractChar();
            if (m_currChar != 'n') {
                Logger::logError(ErrorType::ILLEGAL_SYMBOL, m_currLine, "\\");
            }
            unextractChar();
        }
    }
}

std::vector<Token>& Tokenizer::tokenize() {
    while (m_istream.get(m_currChar)) {
        m_tokenStr.clear();
        m_symbol = SymbolEnum::UNKNOWN;
        m_tokenValue = 0;
        if (skipVacant()) break;
        // 处理标识符
        if (isalpha(m_currChar) || m_currChar == '_') {
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
        } else if (m_currChar == '\'') {
            extractChar();
            std::string charNum = std::to_string(m_currChar);
            m_tokenValue = std::stoi(charNum);
            m_symbol = SymbolEnum::INTCON;
            extractChar();
        }
        if (m_symbol != SymbolEnum::COMMENT) {
            m_currToken = makeToken(m_currLine, m_symbol, m_tokenStr, m_tokenValue);
        }
    }

    return m_tokenList;
}

SymbolEnum Tokenizer::getReservedWordSymbol(std::string& word) {
    static const std::set<std::string> reservedWordSet{
        "const",
        "main",
        "int",
        "char",
        "void",
        "return",
        "if",
        "else",
        "break",
        "continue",
        "while",
        "for",
        "getint",
        "printf",
    };
    std::string copy(word);
    toUpper(copy);
    auto m_symbol = getSymbolEnumByText(copy + "TK");
    return reservedWordSet.count(word) != 0 ? m_symbol : SymbolEnum::IDENFR;
}

void Tokenizer::extractChar() {
    m_istream.get(m_currChar);
}

void Tokenizer::unextractChar() {
    m_istream.unget();
}