#ifndef TOKENIZER_H
#define TOKENIZER_H
#include <fstream>
#include <iostream>
#include <vector>

#include "SymbolEnum.h"

struct Token {
    const SymbolEnum symbol;
    const std::string literal;
    const int value;
    const int lineNum;
    explicit Token(int line, SymbolEnum sym, const std::string& lit, int val) :
        lineNum(line), symbol(sym), value(val), literal(lit) {}

    friend std::ostream& operator<<(std::ostream& os, const Token& token) {
        os << token.lineNum << " " << getSymbolText(token.symbol) << " " << token.literal << "\n";
        return os;
    }
};

class Tokenizer {
public:
    using TokenIter = std::vector<Token>::iterator;

public:
    explicit Tokenizer(std::filebuf& fileBuf);

    std::vector<Token>& tokenize();

private:
    Tokenizer(const Tokenizer&) = delete;
    Tokenizer& operator=(const Tokenizer&) = delete;
    Tokenizer(Tokenizer&&) = delete;
    Tokenizer& operator=(Tokenizer&&) = delete;

private:
    TokenIter makeToken(int lineNum, const SymbolEnum& symbolNum, const std::string& literal, int value = 0);
    SymbolEnum getReservedWordSymbol(std::string& word) const;
    void extractChar();
    void unextractChar();

private:
    std::istream m_istream;
    std::vector<Token> m_tokenList;
    TokenIter m_currToken{nullptr};
    char m_currChar;
    unsigned int m_currLine{1};
};

#endif