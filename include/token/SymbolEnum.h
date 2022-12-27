#ifndef SYMBOL_ENUM_H
#define SYMBOL_ENUM_H
#include <vector>
#include <string>
#include <algorithm>
#include <map>
enum class SymbolEnum : int {
    UNKNOWN = 0,
    IDENFR,  // 标识符
    INTCON,  // 无符号整数
    CHARCON, // 字符
    STRCON,  // 常量字符串
    // 分界符
    SEMICN,  // ;
    COMMA,   // ,
    LPARENT, // (
    RPARENT, // )
    LBRACK,  // [
    RBRACK,  // ]
    LBRACE,  // {
    RBRACE,  // }
    PLUS,    // +
    MINU,    // -
    MULT,    // *
    DIV,     // /
    MOD,     // %
    ASSIGN,  // =
    LSS,     // <
    LEQ,     // <=
    GRE,     // >
    GEQ,     // >=
    EQL,     // ==
    NEQ,     // !=
    NOT,     // !
    AND,     // &&
    OR,      // ||
    COMMENT, // // /*
    // 保留字
    CONSTTK,    // const
    MAINTK,     // main
    INTTK,      // int
    CHARTK,     // char
    VOIDTK,     // void
    RETURNTK,   // return
    IFTK,       // if
    ELSETK,     // else
    BREAKTK,    // break
    CONTINUETK, // continue
    WHILETK,    // while
    FORTK,      // for
    GETINTTK,   // getint
    PRINTFTK,   // printf
};

static const std::vector<std::string> s_symbolText{
    "UNKNOWN",
    "IDENFR",
    "INTCON",
    "CHARCON",
    "STRCON",
    "SEMICN",
    "COMMA",
    "LPARENT",
    "RPARENT",
    "LBRACK",
    "RBRACK",
    "LBRACE",
    "RBRACE",
    "PLUS",
    "MINU",
    "MULT",
    "DIV",
    "MOD",
    "ASSIGN",
    "LSS",
    "LEQ",
    "GRE",
    "GEQ",
    "EQL",
    "NEQ",
    "NOT",
    "AND",
    "OR",
    "COMMENT",
    "CONSTTK",
    "MAINTK",
    "INTTK",
    "CHARTK",
    "VOIDTK",
    "RETURNTK",
    "IFTK",
    "ELSETK",
    "BREAKTK",
    "CONTINUETK",
    "WHILETK",
    "FORTK",
    "GETINTTK",
    "PRINTFTK",
};

static const std::map<std::string, SymbolEnum> s_punctSymbolMap = {

    {";", SymbolEnum::SEMICN},
    {",", SymbolEnum::COMMA},
    {"(", SymbolEnum::LPARENT},
    {")", SymbolEnum::RPARENT},
    {"[", SymbolEnum::LBRACK},
    {"]", SymbolEnum::RBRACK},
    {"{", SymbolEnum::LBRACE},
    {"}", SymbolEnum::RBRACE},
    {"+", SymbolEnum::PLUS},
    {"-", SymbolEnum::MINU},
    {"*", SymbolEnum::MULT},
    {"/", SymbolEnum::DIV},
    {"%", SymbolEnum::MOD},
    {"=", SymbolEnum::ASSIGN},
    {"<", SymbolEnum::LSS},
    {">", SymbolEnum::GRE},
    {"<=", SymbolEnum::LEQ},
    {">=", SymbolEnum::GEQ},
    {"==", SymbolEnum::EQL},
    {"!=", SymbolEnum::NEQ},
    {"!", SymbolEnum::NOT},
    {"&&", SymbolEnum::AND},
    {"||", SymbolEnum::OR},
};

static const std::string& getSymbolText(SymbolEnum symbol) {
    return s_symbolText[static_cast<std::underlying_type<SymbolEnum>::type>(symbol)];
}

static const SymbolEnum getSymbolEnumByText(const std::string& text) {
    auto target = std::find(s_symbolText.begin(), s_symbolText.end(), text);
    if (target != s_symbolText.end()) {
        return SymbolEnum(std::distance(s_symbolText.begin(), target));
    } else {
        return SymbolEnum::UNKNOWN;
    }
}

static const SymbolEnum getSymbolEnumByPunct(const std::string& punct) {
    if (s_punctSymbolMap.count(punct)) {
        return s_punctSymbolMap.at(punct);
    } else {
        return SymbolEnum::UNKNOWN;
    }
}
#endif
