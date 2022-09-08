#ifndef SYMBOL_ENUM_H
#define SYMBOL_ENUM_H
#include <vector>
#include <string>
#include <algorithm>
enum class SymbolEnum : int {
    UNKNOWN = 0,
    IDENT,  // 标识符
    INTCON, // 无符号整数
    STRCON, // 常量字符串
    // 分界符
    SEMICO,  // ;
    COMMA,   // ,
    LPARENT, // (
    RPARENT, // )
    LBRACK,  // [
    RBRACK,  // ]
    LBRACE,  // {
    RBRACE,  // }
    PLUS,    // +
    MINUS,   // -
    MULT,    // *
    DIV,     // /
    MOD,     // %
    ASSIGN,  // =
    LES,     // <
    LEQ,     // <=
    GREAT,   // >
    GEQ,     // >=
    EQ,      // ==
    NEQ,     // !=
    NOT,     // !
    AND,     // &&
    OR,      // ||
    COMMENT, // // /*
    // 保留字
    CONSTRW,    // const
    MAINRW,     // main
    INTRW,      // int
    VOIDRW,     // void
    RETURNRW,   // return
    IFRW,       // if
    ELSERW,     // else
    BREAKRW,    // break
    CONTINUERW, // continue
    WHILERW,    // while
    GETINTRW,   // getint
    PRINTFRW,   // printf
};

static const std::vector<std::string> s_symbolText{
    "UNKNOWN",
    "IDENT",
    "INTCON",
    "STRCON",
    "SEMICO",
    "COMMA",
    "LPARENT",
    "RPARENT",
    "LBRACK",
    "RBRACK",
    "LBRACE",
    "RBRACE",
    "PLUS",
    "MINUS",
    "MULT",
    "DIV",
    "MOD",
    "ASSIGN",
    "LES",
    "LEQ",
    "GREAT",
    "GEQ",
    "EQ",
    "NEQ",
    "NOT",
    "AND",
    "OR",
    "COMMENT",
    "CONSTRW",
    "MAINRW",
    "INTRW",
    "VOIDRW",
    "RETURNRW",
    "IFRW",
    "ELSERW",
    "BREAKRW",
    "CONTINUERW",
    "WHILERW",
    "GETINTRW",
    "PRINTFRW",
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
#endif
