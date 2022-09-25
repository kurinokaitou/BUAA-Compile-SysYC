#ifndef PARSER_H
#define PARSER_H
#include <token/Tokenizer.h>
#include "VNode.h"
class Parser {
public:
    explicit Parser(std::vector<Token>& tokenList);
    void parse();
    std::shared_ptr<VNodeBase> getASTRoot() const;
    void traversalAST(std::filebuf& file);

private:
    std::shared_ptr<VNodeBase> expect(SymbolEnum symbol, int level);
    std::shared_ptr<VNodeBase> expect(std::initializer_list<SymbolEnum> symbolList, int level);
    bool expectAssignment();
    bool expectPureExp();
    bool expectUnaryOp();
    bool expectFuncRParams();
    void postTraversal(std::shared_ptr<VNodeBase> node, std::ostream& os);
    void preTraversal(std::shared_ptr<VNodeBase> node, std::ostream& os);

private:
    std::shared_ptr<VNodeBase> compUnit(int level);     // 编译单元
    std::shared_ptr<VNodeBase> decl(int level);         // 声明
    std::shared_ptr<VNodeBase> constDecl(int level);    // 常量声明
    std::shared_ptr<VNodeBase> bType(int level);        // 基本类型
    std::shared_ptr<VNodeBase> constDef(int level);     // 常量定义
    std::shared_ptr<VNodeBase> constInitVal(int level); // 常量初值
    std::shared_ptr<VNodeBase> constExp(int level);     // 常量表达式
    std::shared_ptr<VNodeBase> varDecl(int level);      // 变量声明
    std::shared_ptr<VNodeBase> varDef(int level);       // 变量定义
    std::shared_ptr<VNodeBase> initVal(int level);      // 变量初值
    std::shared_ptr<VNodeBase> block(int level);        // 语句块
    std::shared_ptr<VNodeBase> blockItem(int level);    // 语句块项
    std::shared_ptr<VNodeBase> stmt(int level);         // 语句
    std::shared_ptr<VNodeBase> lVal(int level);         // 左值
    std::shared_ptr<VNodeBase> exp(int level);          // 表达式
    std::shared_ptr<VNodeBase> cond(int level);         // 条件表达式
    std::shared_ptr<VNodeBase> number(int level);       // 数字
    std::shared_ptr<VNodeBase> primaryExp(int level);   // 基本表达式
    std::shared_ptr<VNodeBase> unaryExp(int level);     // 一元表达式
    std::shared_ptr<VNodeBase> unaryOp(int level);      // 单目运算符
    std::shared_ptr<VNodeBase> addExp(int level);       // 加减模运算
    std::shared_ptr<VNodeBase> mulExp(int level);       // 乘除模运算
    std::shared_ptr<VNodeBase> relExp(int level);       // 关系表达式
    std::shared_ptr<VNodeBase> eqExp(int level);        // 相等性表达式
    std::shared_ptr<VNodeBase> lAndExp(int level);      // 逻辑与表达式
    std::shared_ptr<VNodeBase> lOrExp(int level);       // 逻辑或表达式
    std::shared_ptr<VNodeBase> funcDef(int level);      // 函数定义
    std::shared_ptr<VNodeBase> mainFuncDef(int level);  // 主函数定义
    std::shared_ptr<VNodeBase> funcType(int level);     // 函数类型
    std::shared_ptr<VNodeBase> funcFParams(int level);  // 函数形参表
    std::shared_ptr<VNodeBase> funcFParam(int level);   // 函数形参
    std::shared_ptr<VNodeBase> funcRParams(int level);  // 函数实参表

private:
    std::vector<Token> m_tokenList;
    std::shared_ptr<VNodeBase> m_astRoot;
    Tokenizer::TokenIter m_currToken;
    bool m_probingMode{false};
};
#endif
