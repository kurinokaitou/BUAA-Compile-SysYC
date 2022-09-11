#ifndef PARSER_H
#define PARSER_H
#include <token/Tokenizer.h>
class Parser {
    explicit Parser(const std::vector<Token>& tokenList);
    void parse();

private:
    void compUnit();     // 编译单元
    void decl();         // 声明
    void constDecl();    // 常量声明
    void bType();        // 基本类型
    void constDef();     // 常量定义
    void constInitVal(); // 常量初值
    void constExp();     // 常量表达式
    void varDecl();      // 变量声明
    void varDef();       // 变量定义
    void initVal();      // 变量初值
    void block();        // 语句块
    void blockItem();    // 语句块项
    void stmt();         // 语句
    void lVal();         // 左值
    void exp();          // 表达式
    void cond();         // 条件表达式
    void primaryExp();   // 基本表达式
    void unaryExp();     // 一元表达式
    void unaryOp();      // 单目运算符
    void addExp();       // 加减模运算
    void mulExp();       // 乘除模运算
    void relExp();       // 关系表达式
    void eqExp();        // 相等性表达式
    void lAndExp();      // 逻辑与表达式
    void lOrExp();       // 逻辑或表达式
    void funcDef();      // 函数定义
    void mainFuncDef();  // 主函数定义
    void funcType();     // 函数类型
    void funcFParams();  // 函数形参表
    void funcRParams();  // 函数实参表

private:
    std::vector<Token> m_tokenList;
};
#endif