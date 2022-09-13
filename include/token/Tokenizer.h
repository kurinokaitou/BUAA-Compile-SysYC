#ifndef TOKENIZER_H
#define TOKENIZER_H
#include <fstream>
#include <iostream>
#include <vector>
#include <set>

#include "SymbolEnum.h"

struct Token {
    SymbolEnum symbol;
    std::string literal;
    int value;
    int lineNum;
    explicit Token(int line, SymbolEnum sym, const std::string& lit, int val) :
        lineNum(line), symbol(sym), literal(lit), value(val) {}
    Token() :
        symbol(SymbolEnum::UNKNOWN), literal(""), value(0), lineNum(0){};
    friend std::ostream& operator<<(std::ostream& os, const Token& token) {
        os << getSymbolText(token.symbol) << " ";
        if (token.symbol == SymbolEnum::INTCON) {
            os << token.value << "\n";
        } else {
            os << token.literal << "\n";
        }
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
    bool skipVacant();
    SymbolEnum skipComment();
    SymbolEnum readIDENFR();
    SymbolEnum readInteger();
    SymbolEnum readPunct();
    SymbolEnum readString();
    void extractChar();
    void unextractChar();
    void expect(const char c);

private:
    std::istream m_istream;
    std::vector<Token> m_tokenList;
    TokenIter m_currToken{nullptr};
    char m_currChar;
    unsigned int m_currLine{1};
    SymbolEnum m_symbol{SymbolEnum::UNKNOWN};
    std::string m_tokenStr;
    int m_tokenValue{0};
};

#endif