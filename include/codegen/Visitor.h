#ifndef VISITOR_H
#define VISITOR_H
#include <symbol/SymbolTable.h>
#include <symbol/ValueType.h>
#include <grammar/VNode.h>
#include <ir/IR.h>
#define MAKE_INT_VAR() m_table.makeItem<VarItem<IntType>>({})
#define MAKE_CHAR_VAR() m_table.makeItem<VarItem<CharType>>({})
#define MAKE_VOID_VAR() m_table.makeItem<VarItem<VoidType>>({})
#define MAKE_INT_ARRAY(dims) m_table.makeItem<VarItem<ArrayType<IntType>>>({{{{}, dims}}, {{{}, dims}}, false})
#define MAKE_CHAR_ARRAY(dims) m_table.makeItem<VarItem<ArrayType<CharType>>>({{{{}, dims}}, {{{}, dims}}, false})

#define MAKE_VAR() m_table.makeItem<VarItem<Type>>({})
#define MAKE_CONST_VAR() m_table.makeItem<ConstVarItem<Type>>(0)

class Visitor {
public:
    explicit Visitor(std::shared_ptr<VNodeBase> astRoot, SymbolTable& table, CodeContext& ctx);
    void visit();

private:
    bool expect(std::shared_ptr<VNodeBase> node, VNodeEnum nodeEnum);
    bool expect(std::shared_ptr<VNodeBase> node, SymbolEnum symbolEnum);
    void compUnit(std::shared_ptr<VNodeBase> node);  // 编译单元
    void decl(std::shared_ptr<VNodeBase> node);      // 声明
    void constDecl(std::shared_ptr<VNodeBase> node); // 常量声明
    template <typename Type>
    void constDef(std::shared_ptr<VNodeBase> node); // 常量定义
    template <typename Type>
    typename Type::InternalType constInitVal(std::shared_ptr<VNodeBase> node, std::vector<size_t>& dims, int level); // 常量初值
    template <typename Type>
    typename ArrayType<Type>::InternalType constInitValArray(std::shared_ptr<VNodeBase> node, std::vector<size_t>& dims, int level);
    template <typename Type>
    typename Type::InternalType constExp(std::shared_ptr<VNodeBase> node); // 常量表达式
    void varDecl(std::shared_ptr<VNodeBase> node);                         // 变量声明
    template <typename Type>
    void varDef(std::shared_ptr<VNodeBase> node); // 变量定义
    template <typename Type>
    typename Type::InternalType initValGlobal(std::shared_ptr<VNodeBase> node, std::vector<size_t>& dims, int level); // 变量初值
    template <typename Type>
    typename ArrayType<Type>::InternalType initValGlobalArray(std::shared_ptr<VNodeBase> node, std::vector<size_t>& dims, int level);
    template <typename Type>
    typename Type::InternalItem initVal(std::shared_ptr<VNodeBase> node, std::vector<size_t>& dims, int level);
    template <typename Type>
    typename ArrayType<Type>::InternalItem initValArray(std::shared_ptr<VNodeBase> node, std::vector<size_t>& dims, int level);

    template <typename Type>
    SymbolTableItem* exp(std::shared_ptr<VNodeBase> node);  // 表达式
    void block(std::shared_ptr<VNodeBase> node);            // 语句块
    void blockItem(std::shared_ptr<VNodeBase> node);        // 语句块项
    void stmt(std::shared_ptr<VNodeBase> node);             // 语句
    SymbolTableItem* lVal(std::shared_ptr<VNodeBase> node); // 左值
    template <typename Type>
    SymbolTableItem* rVal(std::shared_ptr<VNodeBase> node); // 右值（生成式中依然是左值，只不过是右值的功能）
    void cond(std::shared_ptr<VNodeBase> node);             // 条件表达式
    template <typename Type>
    ConstVarItem<Type>* number(std::shared_ptr<VNodeBase> node); // 数字
    template <typename Type>
    SymbolTableItem* primaryExp(std::shared_ptr<VNodeBase> node); // 基本表达式
    template <typename Type>
    SymbolTableItem* unaryExp(std::shared_ptr<VNodeBase> node); // 一元表达式
    SymbolEnum unaryOp(std::shared_ptr<VNodeBase> node);        // 单目运算符
    template <typename Type>
    SymbolTableItem* addExp(std::shared_ptr<VNodeBase> node); // 加减模运算
    template <typename Type>
    SymbolTableItem* mulExp(std::shared_ptr<VNodeBase> node); // 乘除模运算
    template <typename Type>
    SymbolTableItem* relExp(std::shared_ptr<VNodeBase> node); // 关系表达式
    template <typename Type>
    SymbolTableItem* eqExp(std::shared_ptr<VNodeBase> node); // 相等性表达式
    template <typename Type>
    void lAndExp(std::shared_ptr<VNodeBase> node); // 逻辑与表达式
    template <typename Type>
    void lOrExp(std::shared_ptr<VNodeBase> node);                                                            // 逻辑或表达式
    void funcDef(std::shared_ptr<VNodeBase> node);                                                           // 函数定义
    void mainFuncDef(std::shared_ptr<VNodeBase> node);                                                       // 主函数定义
    ValueTypeEnum funcType(std::shared_ptr<VNodeBase> node);                                                 // 函数类型
    std::vector<SymbolTableItem*> funcFParams(std::shared_ptr<VNodeBase> node);                              // 函数形参表
    SymbolTableItem* funcFParam(std::shared_ptr<VNodeBase> node);                                            // 函数形参
    std::vector<SymbolTableItem*> funcRParams(std::shared_ptr<VNodeBase> node, FuncItem* func, int lineNum); // 函数实参表
    template <typename Type>
    SymbolTableItem* funcRParam(std::shared_ptr<VNodeBase> node, SymbolTableItem* formalParam);
    ValueTypeEnum bType(std::shared_ptr<VNodeBase> node); // 基本类型

private:
    template <typename Type>
    std::pair<typename Type::InternalType, bool> calConstExp(std::shared_ptr<VNodeBase> node); // 计算常量表达式
    SymbolTableItem* makeTempItem(ValueTypeEnum type, bool isArray = false, std::vector<size_t>&& dims = {});
    template <typename Type>
    bool checkConvertiable(SymbolTableItem* item);

private:
    SymbolTable& m_table;
    CodeContext& m_ctx;
    std::shared_ptr<VNodeBase> m_astRoot;
};

#endif