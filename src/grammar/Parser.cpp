#include <grammar/Parser.h>
#include <Log.h>

Parser::Parser(std::vector<Token>& tokenList) {
    m_tokenList.emplace_back();
    m_tokenList.insert(m_tokenList.end(), tokenList.begin(), tokenList.end());
    m_currToken = m_tokenList.begin();
}

std::shared_ptr<VNodeBase> Parser::parse() {
    auto compUnitNode = compUnit(0);
    if (m_currToken != m_tokenList.end()) {
        PARSER_LOG_INFO("Token解析未完成");
    }
    return compUnitNode;
}

// 期望获取symbol对应的内容，并返回生成的叶节点
std::shared_ptr<VNodeBase> Parser::expect(SymbolEnum symbol) {
    if ((m_currToken + 1)->symbol == symbol) {
        m_currToken++;
        return std::make_shared<VNodeLeaf>(symbol, *m_currToken);
    } else {
        PARSER_LOG_ERROR(getSymbolText(symbol) + " error");
        return std::make_shared<VNodeLeaf>(symbol, *(m_currToken - 1), false);
    }
}

// 期望获取symbolList包含的内容，并返回生成的叶节点
std::shared_ptr<VNodeBase> Parser::expect(std::initializer_list<SymbolEnum> symbolList) {
    std::set<SymbolEnum> symset(symbolList);
    if (symset.count((m_currToken + 1)->symbol)) {
        m_currToken++;
        return std::make_shared<VNodeLeaf>(m_currToken->symbol, *m_currToken);
    } else {
        PARSER_LOG_ERROR(getSymbolText(m_currToken->symbol) + " error");
        return std::make_shared<VNodeLeaf>(*symbolList.begin(), *(m_currToken - 1), false);
    }
}

// 编译单元compUnit -> {decl} {funcDef} mainFuncDef
std::shared_ptr<VNodeBase> Parser::compUnit(int level) {
    std::vector<std::shared_ptr<VNodeBase>> children;
    auto compUnitNode = std::make_shared<VNodeBranch>(VNodeEnum::COMPUNIT);
    compUnitNode->setLevel(level);
    // 获取 decl, 只要不是 void|int func()的形式就可以按照decl去读取
    while ((m_currToken + 3)->symbol != SymbolEnum::LPARENT) {
        auto child = decl(level + 1);
        children.push_back(std::move(child));
    }
    // 获取 funcDef, 只要不是 void|int main () 的形式就可以按照funcDef去读取
    while ((m_currToken + 2)->symbol != SymbolEnum::MAINRW) {
        auto child = funcDef(level + 1);
        children.push_back(std::move(child));
    }
    // 获取 mainFuncDef
    auto child = mainFuncDef(level + 1);
    children.push_back(std::move(child));
    for (auto& child : children) {
        compUnitNode->addChild(std::move(child));
    }
    return compUnitNode;
}
// 声明decl -> constDef | varDef
std::shared_ptr<VNodeBase> Parser::decl(int level) {
    auto declNode = std::make_shared<VNodeBranch>(VNodeEnum::DECL);
    declNode->setLevel(level);
    std::shared_ptr<VNodeBase> child;
    if ((m_currToken + 1)->symbol == SymbolEnum::CONSTRW) {
        child = constDecl(level + 1);
    } else {
        child = varDecl(level + 1);
    }
    declNode->addChild(std::move(child));
    return declNode;
}
// TODO: 完成所有的编译项
// 常量声明constDecl -> 'const' bType constDef {',' constDef}';'
std::shared_ptr<VNodeBase> Parser::constDecl(int level) {
    std::vector<std::shared_ptr<VNodeBase>> children;
    auto constDeclNode = std::make_shared<VNodeBranch>(VNodeEnum::CONSTDECL);
    constDeclNode->setLevel(level);
    children.push_back(expect(SymbolEnum::CONSTRW));
    children.push_back(bType(level + 1));
    children.push_back(constDef(level + 1));
    while ((m_currToken + 1)->symbol == SymbolEnum::COMMA) {
        children.push_back(expect(SymbolEnum::COMMA));
        children.push_back(constDef(level + 1));
    }
    children.push_back(expect(SymbolEnum::SEMICO));
    for (auto& child : children) {
        constDeclNode->addChild(std::move(child));
    }
    return constDeclNode;
}

// 基本类型bType -> 'int'
std::shared_ptr<VNodeBase> Parser::bType(int level) {
    auto btypeNode = std::make_shared<VNodeBranch>(VNodeEnum::BTYPE);
    btypeNode->setLevel(level);
    btypeNode->addChild(expect(SymbolEnum::INTRW));
    return btypeNode;
}

// 常量定义constDef -> Ident {'[' constExp ']'} '=' constInitVal;
std::shared_ptr<VNodeBase> Parser::constDef(int level) {
    std::vector<std::shared_ptr<VNodeBase>> children;
    auto constDefNode = std::make_shared<VNodeBranch>(VNodeEnum::DECL);
    constDefNode->setLevel(level);
    children.push_back(expect(SymbolEnum::IDENT));
    while ((m_currToken + 1)->symbol == SymbolEnum::LBRACK) {
        children.push_back(expect(SymbolEnum::LBRACK));
        children.push_back(constExp(level + 1));
        children.push_back(expect(SymbolEnum::RBRACK));
    }
    children.push_back(expect(SymbolEnum::ASSIGN));
    children.push_back(constInitVal(level + 1));
    for (auto& child : children) {
        constDefNode->addChild(std::move(child));
    }
    return constDefNode;
}
// 常量初值 constInitVal -> constExp | '{' [ constInitVal { ',' constInitVal } ] '}'
std::shared_ptr<VNodeBase> Parser::constInitVal(int level) {
    auto constInitValNode = std::make_shared<VNodeBranch>(VNodeEnum::DECL);
    constInitValNode->setLevel(level);
    if ((m_currToken + 1)->symbol == SymbolEnum::LBRACE) {
        std::vector<std::shared_ptr<VNodeBase>> children;
        children.push_back(expect(SymbolEnum::LBRACE));
        if ((m_currToken + 1)->symbol != SymbolEnum::RBRACE) {
            children.push_back(constInitVal(level + 1));
            while ((m_currToken + 1)->symbol == SymbolEnum::COMMA) {
                children.push_back(expect(SymbolEnum::COMMA));
                children.push_back(constInitVal(level + 1));
            }
        }
        children.push_back(expect(SymbolEnum::RBRACE));
        for (auto& child : children) {
            constInitValNode->addChild(std::move(child));
        }
    } else {
        constInitValNode->addChild(constExp(level + 1));
    }
    return constInitValNode;
}
// 常量表达式 constExp -> addExp
std::shared_ptr<VNodeBase> Parser::constExp(int level) {
    auto constExpNode = std::make_shared<VNodeBranch>(VNodeEnum::DECL);
    constExpNode->setLevel(level);
    constExpNode->addChild(addExp(level + 1));
    return constExpNode;
}

// 变量声明
std::shared_ptr<VNodeBase> Parser::varDecl(int level) {
    auto declNode = std::make_shared<VNodeBranch>(VNodeEnum::DECL);
    m_currToken++;
    return declNode;
}
// 变量定义
std::shared_ptr<VNodeBase> Parser::varDef(int level) {
    auto declNode = std::make_shared<VNodeBranch>(VNodeEnum::DECL);
    m_currToken++;
    return declNode;
}
// 变量初值
std::shared_ptr<VNodeBase> Parser::initVal(int level) {
    auto declNode = std::make_shared<VNodeBranch>(VNodeEnum::DECL);
    m_currToken++;
    return declNode;
}
// 语句块
std::shared_ptr<VNodeBase> Parser::block(int level) {
    auto declNode = std::make_shared<VNodeBranch>(VNodeEnum::DECL);
    m_currToken++;
    return declNode;
}
// 语句块项
std::shared_ptr<VNodeBase> Parser::blockItem(int level) {
    auto declNode = std::make_shared<VNodeBranch>(VNodeEnum::DECL);
    m_currToken++;
    return declNode;
}
// 语句
std::shared_ptr<VNodeBase> Parser::stmt(int level) {
    auto declNode = std::make_shared<VNodeBranch>(VNodeEnum::DECL);
    m_currToken++;
    return declNode;
}
// 左值
std::shared_ptr<VNodeBase> Parser::lVal(int level) {
    auto declNode = std::make_shared<VNodeBranch>(VNodeEnum::DECL);
    m_currToken++;
    return declNode;
}
// 表达式
std::shared_ptr<VNodeBase> Parser::exp(int level) {
    auto declNode = std::make_shared<VNodeBranch>(VNodeEnum::DECL);
    m_currToken++;
    return declNode;
}
// 条件表达式
std::shared_ptr<VNodeBase> Parser::cond(int level) {
    auto declNode = std::make_shared<VNodeBranch>(VNodeEnum::DECL);
    m_currToken++;
    return declNode;
}
// 基本表达式
std::shared_ptr<VNodeBase> Parser::primaryExp(int level) {
    auto declNode = std::make_shared<VNodeBranch>(VNodeEnum::DECL);
    m_currToken++;
    return declNode;
}
// 一元表达式
std::shared_ptr<VNodeBase> Parser::unaryExp(int level) {
    auto declNode = std::make_shared<VNodeBranch>(VNodeEnum::DECL);
    m_currToken++;
    return declNode;
}
// 单目运算符
std::shared_ptr<VNodeBase> Parser::unaryOp(int level) {
    auto declNode = std::make_shared<VNodeBranch>(VNodeEnum::DECL);
    m_currToken++;
    return declNode;
}
// 加减模运算
std::shared_ptr<VNodeBase> Parser::addExp(int level) {
    auto declNode = std::make_shared<VNodeBranch>(VNodeEnum::DECL);
    m_currToken++;
    return declNode;
}
// 乘除模运算
std::shared_ptr<VNodeBase> Parser::mulExp(int level) {
    auto declNode = std::make_shared<VNodeBranch>(VNodeEnum::DECL);
    m_currToken++;
    return declNode;
}
// 关系表达式
std::shared_ptr<VNodeBase> Parser::relExp(int level) {
    auto declNode = std::make_shared<VNodeBranch>(VNodeEnum::DECL);
    m_currToken++;
    return declNode;
}
// 相等性表达式
std::shared_ptr<VNodeBase> Parser::eqExp(int level) {
    auto declNode = std::make_shared<VNodeBranch>(VNodeEnum::DECL);
    m_currToken++;
    return declNode;
}
// 逻辑与表达式
std::shared_ptr<VNodeBase> Parser::lAndExp(int level) {
    auto declNode = std::make_shared<VNodeBranch>(VNodeEnum::DECL);
    m_currToken++;
    return declNode;
}
// 逻辑或表达式
std::shared_ptr<VNodeBase> Parser::lOrExp(int level) {
    auto declNode = std::make_shared<VNodeBranch>(VNodeEnum::DECL);
    m_currToken++;
    return declNode;
}
// 函数定义
std::shared_ptr<VNodeBase> Parser::funcDef(int level) {
    auto declNode = std::make_shared<VNodeBranch>(VNodeEnum::DECL);
    m_currToken++;
    return declNode;
}
// 主函数定义
std::shared_ptr<VNodeBase> Parser::mainFuncDef(int level) {
    auto declNode = std::make_shared<VNodeBranch>(VNodeEnum::DECL);
    m_currToken++;
    return declNode;
}
// 函数类型
std::shared_ptr<VNodeBase> Parser::funcType(int level) {
    auto declNode = std::make_shared<VNodeBranch>(VNodeEnum::DECL);
    m_currToken++;
    return declNode;
}
// 函数形参表
std::shared_ptr<VNodeBase> Parser::funcFParams(int level) {
    auto declNode = std::make_shared<VNodeBranch>(VNodeEnum::DECL);
    m_currToken++;
    return declNode;
}
// 函数实参表
std::shared_ptr<VNodeBase> Parser::funcRParams(int level) {
    auto declNode = std::make_shared<VNodeBranch>(VNodeEnum::DECL);
    m_currToken++;
    return declNode;
}