#include <grammar/Parser.h>

Parser::Parser(const std::vector<Token>& tokenList) :
    m_tokenList(tokenList) {
}
// 编译单元compUnit -> {decl} {funcDef} mainFuncDef
void Parser::compUnit() {
}
// 声明decl -> constDef | varDef
void Parser::decl() {
}
// 常量声明constDecl -> 'const' bType constDef {',' constDef}'{}'
void Parser::constDecl() {
}
// 基本类型bType -> 'int'
void Parser::bType() {
}
// 常量定义constDef -> Ident {'[' constExp ']'} '=' constInitVal{}
void Parser::constDef() {
}
// 常量初值
void Parser::constInitVal() {
}
// 常量表达式
void Parser::constExp() {
}
// 变量声明
void Parser::varDecl() {
}
// 变量定义
void Parser::varDef() {
}
// 变量初值
void Parser::initVal() {
}
// 语句块
void Parser::block() {
}
// 语句块项
void Parser::blockItem() {
}
// 语句
void Parser::stmt() {
}
// 左值
void Parser::lVal() {
}
// 表达式
void Parser::exp() {
}
// 条件表达式
void Parser::cond() {
}
// 基本表达式
void Parser::primaryExp() {
}
// 一元表达式
void Parser::unaryExp() {
}
// 单目运算符
void Parser::unaryOp() {
}
// 加减模运算
void Parser::addExp() {
}
// 乘除模运算
void Parser::mulExp() {
}
// 关系表达式
void Parser::relExp() {
}
// 相等性表达式
void Parser::eqExp() {
}
// 逻辑与表达式
void Parser::lAndExp() {
}
// 逻辑或表达式
void Parser::lOrExp() {
}
// 函数定义
void Parser::funcDef() {
}
// 主函数定义
void Parser::mainFuncDef() {
}
// 函数类型
void Parser::funcType() {
}
// 函数形参表
void Parser::funcFParams() {
}
// 函数实参表
void Parser::funcRParams() {
}