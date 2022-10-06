#ifndef VISITOR_H
#define VISITOR_H
#include <symbol/SymbolTable.h>
#include <symbol/ValueType.h>
#include <grammar/VNode.h>

class Visitor {
public:
    explicit Visitor(std::shared_ptr<VNodeBase> astRoot, SymbolTable& table);
    void visit();

private:
    bool expect(std::shared_ptr<VNodeBase> node, VNodeEnum nodeEnum);
    bool expect(std::shared_ptr<VNodeBase> node, SymbolEnum symbolEnum);
    void compUnit(std::shared_ptr<VNodeBase> node);  // 编译单元
    void decl(std::shared_ptr<VNodeBase> node);      // 声明
    void constDecl(std::shared_ptr<VNodeBase> node); // 常量声明
    void constDef(std::shared_ptr<VNodeBase> node);  // 常量定义
    template <typename VT>
    typename VT::InternalType constInitVal(std::shared_ptr<VNodeBase> node, std::vector<size_t>& dims, int level) { // 常量初值
        return typename VT::InternalType();
    };
    IntType::InternalType constExp(std::shared_ptr<VNodeBase> node); // 常量表达式

    void varDecl(std::shared_ptr<VNodeBase> node); // 变量声明
    void varDef(std::shared_ptr<VNodeBase> node);  // 变量定义
    template <typename VT>
    typename VT::InternalType initValGlobal(std::shared_ptr<VNodeBase> node, std::vector<size_t>& dims, int level) { // 变量初值
        return typename VT::InternalType();
    }
    template <typename VT>
    typename VT::InternalItem initVal(std::shared_ptr<VNodeBase> node, std::vector<size_t>& dims, int level) {
        return typename VT::InternalItem();
    }

    VarItem<IntType>* exp(std::shared_ptr<VNodeBase> node);                     // 表达式
    void block(std::shared_ptr<VNodeBase> node);                                // 语句块
    void blockItem(std::shared_ptr<VNodeBase> node);                            // 语句块项
    void stmt(std::shared_ptr<VNodeBase> node);                                 // 语句
    void lVal(std::shared_ptr<VNodeBase> node);                                 // 左值
    void cond(std::shared_ptr<VNodeBase> node);                                 // 条件表达式
    void number(std::shared_ptr<VNodeBase> node);                               // 数字
    void primaryExp(std::shared_ptr<VNodeBase> node);                           // 基本表达式
    void unaryExp(std::shared_ptr<VNodeBase> node);                             // 一元表达式
    void unaryOp(std::shared_ptr<VNodeBase> node);                              // 单目运算符
    void addExp(std::shared_ptr<VNodeBase> node);                               // 加减模运算
    void mulExp(std::shared_ptr<VNodeBase> node);                               // 乘除模运算
    void relExp(std::shared_ptr<VNodeBase> node);                               // 关系表达式
    void eqExp(std::shared_ptr<VNodeBase> node);                                // 相等性表达式
    void lAndExp(std::shared_ptr<VNodeBase> node);                              // 逻辑与表达式
    void lOrExp(std::shared_ptr<VNodeBase> node);                               // 逻辑或表达式
    void funcDef(std::shared_ptr<VNodeBase> node);                              // 函数定义
    void mainFuncDef(std::shared_ptr<VNodeBase> node);                          // 主函数定义
    ValueTypeEnum funcType(std::shared_ptr<VNodeBase> node);                    // 函数类型
    std::vector<SymbolTableItem*> funcFParams(std::shared_ptr<VNodeBase> node); // 函数形参表
    SymbolTableItem* funcFParam(std::shared_ptr<VNodeBase> node);               // 函数形参
    void funcRParams(std::shared_ptr<VNodeBase> node);                          // 函数实参表
    ValueTypeEnum bType(std::shared_ptr<VNodeBase> node);                       // 基本类型

    IntType::InternalType calConstExp(std::shared_ptr<VNodeBase> node); // 计算常量表达式

private:
    SymbolTable& m_table;
    std::shared_ptr<VNodeBase> m_astRoot;
};

#endif