#ifndef TOKENIZER_H
#define TOKENIZER_H
#include <fstream>
#include <iostream>
#include <memory>
#include <vector>

#include "SymbolEnum.h"

struct Token {
    const SymbolEnum symbol;
    const std::string literal;
    const int value;
    const int lineNum;
    explicit Token(int line, SymbolEnum sym, int val, const std::string& lit) :
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
    explicit Tokenizer(std::string& sourcePath);
    Tokenizer(Tokenizer&&) = default;
    Tokenizer& operator=(Tokenizer&&) = default;
    std::vector<Token>& tokenize();

private:
    Tokenizer(const Tokenizer&) = delete;
    Tokenizer& operator=(const Tokenizer&) = delete;

private:
    TokenIter makeToken(int lineNum, const SymbolEnum& symbolNum, const std::string& literal, int value = 0);

private:
    std::unique_ptr<std::istream> m_istream;
    std::vector<Token> m_tokensList;
    TokenIter m_currentToken{nullptr};
};

#endif