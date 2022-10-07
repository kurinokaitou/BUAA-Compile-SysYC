#ifndef ERROR_H
#define ERROR_H
#include <string>
enum class ErrorType : char {
    UNKNOWN = 'U',
    // 词法分析
    ILLEGAL_SYMBOL = 'a',
    // 语义分析 符号表
    REDEF_IDENT = 'b',
    UNDECL_IDENT = 'c',
    PARAMS_NUM_NOT_MATCH = 'd',
    PARAMS_TYPE_NOT_MATCH = 'e',
    VOID_FUNC_HAVE_RETURNED = 'f',
    NONVOID_FUNC_MISS_RETURN = 'g',
    ASSIGN_TO_CONST = 'h',
    // 语法分析
    MISSING_SEMICN = 'i',
    MISSING_RPARENT = 'j',
    MISSING_RBRACK = 'k',
    // 其他
    PRINTF_UMATCHED = 'l',
    BRK_CONT_NOT_IN_LOOP = 'm'
};

static std::string genErrorText(ErrorType error, const std::string& meta, const std::string& target) {
    switch (error) {
    case ErrorType::ILLEGAL_SYMBOL:
        return "Illegal symbol '" + meta + "' used in format string";
    case ErrorType::REDEF_IDENT:
        return "Redefinition of '" + meta + "'";
    case ErrorType::UNDECL_IDENT:
        return "Use of undeclared identifier '" + meta + "'";
    case ErrorType::PARAMS_NUM_NOT_MATCH:
        return "Number of function's parameters not match, expected " + target + ", but " + meta + " provided";
    case ErrorType::PARAMS_TYPE_NOT_MATCH:
        return "Type of function's parameters not match";
    case ErrorType::VOID_FUNC_HAVE_RETURNED:
        return "Void function '" + meta + "' should not return a value";
    case ErrorType::NONVOID_FUNC_MISS_RETURN:
        return "Non-void function '" + meta + "' does not return a value";
    case ErrorType::ASSIGN_TO_CONST:
        return "Cannot assign to const variable '" + meta + "'";
    case ErrorType::MISSING_SEMICN:
        return "Expected ';'";
    case ErrorType::MISSING_RPARENT:
        return "Expected ')'";
    case ErrorType::MISSING_RBRACK:
        return "Expected ']'";
    case ErrorType::PRINTF_UMATCHED:
        return "Printf format string expected " + target + " data arguments but " + meta + " provided";
    case ErrorType::BRK_CONT_NOT_IN_LOOP:
        return "Break or continue statement not in loop";
    case ErrorType::UNKNOWN:
    default: return "";
    }
}

#endif