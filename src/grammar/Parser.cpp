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

bool Parser::expectAssignment() {
    auto iterBefore = m_currToken;
    auto temp = lVal(-1);
    if (temp->isCorrect()) {
        auto assign = expect(SymbolEnum::ASSIGN);
        m_currToken = iterBefore;
        return assign->isCorrect();
    } else {
        return false;
    }
}
bool Parser::expectPureExp() {
    auto iterBefore = m_currToken;
    auto temp = exp(-1);
    if (temp->isCorrect() && m_currToken != iterBefore) {
        auto semi = expect(SymbolEnum::SEMICO);
        m_currToken = iterBefore;
        return semi->isCorrect();
    } else {
        return false;
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
        child = constDecl(level);
    } else {
        child = varDecl(level);
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
    children.push_back(bType(level));
    children.push_back(constDef(level));
    while ((m_currToken + 1)->symbol == SymbolEnum::COMMA) {
        children.push_back(expect(SymbolEnum::COMMA));
        children.push_back(constDef(level));
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
    auto constDefNode = std::make_shared<VNodeBranch>(VNodeEnum::CONSTDEF);
    constDefNode->setLevel(level);
    children.push_back(expect(SymbolEnum::IDENT));
    while ((m_currToken + 1)->symbol == SymbolEnum::LBRACK) {
        children.push_back(expect(SymbolEnum::LBRACK));
        children.push_back(constExp(level));
        children.push_back(expect(SymbolEnum::RBRACK));
    }
    children.push_back(expect(SymbolEnum::ASSIGN));
    children.push_back(constInitVal(level));
    for (auto& child : children) {
        constDefNode->addChild(std::move(child));
    }
    return constDefNode;
}
// 常量初值 constInitVal -> constExp | '{' [ constInitVal { ',' constInitVal } ] '}'
std::shared_ptr<VNodeBase> Parser::constInitVal(int level) {
    auto constInitValNode = std::make_shared<VNodeBranch>(VNodeEnum::CONSTINITVAL);
    constInitValNode->setLevel(level);
    if ((m_currToken + 1)->symbol == SymbolEnum::LBRACE) {
        std::vector<std::shared_ptr<VNodeBase>> children;
        children.push_back(expect(SymbolEnum::LBRACE));
        if ((m_currToken + 1)->symbol != SymbolEnum::RBRACE) {
            children.push_back(constInitVal(level));
            while ((m_currToken + 1)->symbol == SymbolEnum::COMMA) {
                children.push_back(expect(SymbolEnum::COMMA));
                children.push_back(constInitVal(level));
            }
        }
        children.push_back(expect(SymbolEnum::RBRACE));
        for (auto& child : children) {
            constInitValNode->addChild(std::move(child));
        }
    } else {
        constInitValNode->addChild(constExp(level));
    }
    return constInitValNode;
}
// 常量表达式 constExp -> addExp
std::shared_ptr<VNodeBase> Parser::constExp(int level) {
    auto constExpNode = std::make_shared<VNodeBranch>(VNodeEnum::CONSTEXP);
    constExpNode->setLevel(level);
    constExpNode->addChild(addExp(level));
    return constExpNode;
}

// 变量声明 bType -> varDef { ',' varDef } ';'
std::shared_ptr<VNodeBase> Parser::varDecl(int level) {
    std::vector<std::shared_ptr<VNodeBase>> children;
    auto varDeclNode = std::make_shared<VNodeBranch>(VNodeEnum::VARDECL);
    varDeclNode->setLevel(level);
    children.push_back(bType(level));
    children.push_back(varDef(level));
    while ((m_currToken + 1)->symbol == SymbolEnum::COMMA) {
        children.push_back(expect(SymbolEnum::COMMA));
        children.push_back(varDef(level));
    }
    children.push_back(expect(SymbolEnum::SEMICO));
    for (auto& child : children) {
        varDeclNode->addChild(std::move(child));
    }
    return varDeclNode;
}
// 变量定义 varDef -> ident { '[' constExp ']' } [ '=' initVal ]
std::shared_ptr<VNodeBase> Parser::varDef(int level) {
    std::vector<std::shared_ptr<VNodeBase>> children;
    auto varDefNode = std::make_shared<VNodeBranch>(VNodeEnum::VARDEF);
    varDefNode->setLevel(level);
    children.push_back(expect(SymbolEnum::IDENT));
    while ((m_currToken + 1)->symbol == SymbolEnum::LBRACK) {
        children.push_back(expect(SymbolEnum::LBRACK));
        children.push_back(constExp(level));
        children.push_back(expect(SymbolEnum::RBRACK));
    }
    if ((m_currToken + 1)->symbol == SymbolEnum::ASSIGN) {
        children.push_back(expect(SymbolEnum::ASSIGN));
        children.push_back(initVal(level));
    }

    for (auto& child : children) {
        varDefNode->addChild(std::move(child));
    }
    return varDefNode;
}
// 变量初值 initVal -> exp | '{' [ initVal { ',' initVal } ] '}'
std::shared_ptr<VNodeBase> Parser::initVal(int level) {
    auto initValNode = std::make_shared<VNodeBranch>(VNodeEnum::INITVAL);
    initValNode->setLevel(level);
    if ((m_currToken + 1)->symbol == SymbolEnum::LBRACE) {
        std::vector<std::shared_ptr<VNodeBase>> children;
        children.push_back(expect(SymbolEnum::LBRACE));
        if ((m_currToken + 1)->symbol != SymbolEnum::RBRACE) {
            children.push_back(initVal(level));
            while ((m_currToken + 1)->symbol != SymbolEnum::COMMA) {
                children.push_back(expect(SymbolEnum::COMMA));
                children.push_back(initVal(level));
            }
        }
        children.push_back(expect(SymbolEnum::RBRACE));
        for (auto& child : children) {
            initValNode->addChild(std::move(child));
        }
    } else {
        initValNode->addChild(exp(level));
    }
    return initValNode;
}

// 语句块 block -> '{' { blockItem } '}'
std::shared_ptr<VNodeBase> Parser::block(int level) {
    std::vector<std::shared_ptr<VNodeBase>> children;
    auto blockNode = std::make_shared<VNodeBranch>(VNodeEnum::BLOCK);
    blockNode->setLevel(level);
    children.push_back(expect(SymbolEnum::LBRACE));
    while ((m_currToken + 1)->symbol != SymbolEnum::RBRACE) {
        children.push_back(blockItem(level + 1));
    }
    children.push_back(expect(SymbolEnum::RBRACE));
    for (auto& child : children) {
        blockNode->addChild(std::move(child));
    }
    return blockNode;
}

// 语句块项 blockItem -> decl | stmt
std::shared_ptr<VNodeBase> Parser::blockItem(int level) {
    auto blockItemNode = std::make_shared<VNodeBranch>(VNodeEnum::BLOCKITEM);
    blockItemNode->setLevel(level);
    if ((m_currToken + 1)->symbol == SymbolEnum::CONSTRW || (m_currToken + 1)->symbol == SymbolEnum::INTRW) {
        blockItemNode->addChild(decl(level));
    } else {
        blockItemNode->addChild(stmt(level));
    }
    return blockItemNode;
}

/**
 * 语句 stmt -> lVal '=' exp ';'
                | lVal '=' 'getint''('')'';' 
                | [exp] ';'
                | block 
                | 'if' '(' cond ')' stmt [ 'else' stmt ] 
                | 'while' '(' cond ')' stmt 
                | 'break' ';' 
                | 'continue' ';' 
                | 'return' [exp] ';'
                | 'printf''('formatString{','exp}')'';'
 */
std::shared_ptr<VNodeBase> Parser::stmt(int level) {
    std::vector<std::shared_ptr<VNodeBase>> children;
    auto stmtNode = std::make_shared<VNodeBranch>(VNodeEnum::STMT);
    stmtNode->setLevel(level);
    // 首先是开头具有标识符的情况，包括 if/while/break/continue/return/printf
    if ((m_currToken + 1)->symbol == SymbolEnum::IFRW) {
        children.push_back(expect(SymbolEnum::IFRW));
        children.push_back(expect(SymbolEnum::LPARENT));
        children.push_back(cond(level));
        children.push_back(expect(SymbolEnum::RPARENT));
        children.push_back(stmt(level));
        if ((m_currToken + 1)->symbol == SymbolEnum::ELSERW) {
            children.push_back(expect(SymbolEnum::ELSERW));
            children.push_back(stmt(level));
        }
    } else if ((m_currToken + 1)->symbol == SymbolEnum::WHILERW) {
        children.push_back(expect(SymbolEnum::WHILERW));
        children.push_back(expect(SymbolEnum::LPARENT));
        children.push_back(cond(level));
        children.push_back(expect(SymbolEnum::RPARENT));
        children.push_back(stmt(level));
    } else if ((m_currToken + 1)->symbol == SymbolEnum::BREAKRW || (m_currToken + 1)->symbol == SymbolEnum::CONTINUERW) {
        children.push_back(expect({SymbolEnum::BREAKRW, SymbolEnum::CONTINUERW}));
        children.push_back(expect(SymbolEnum::SEMICO));
    } else if ((m_currToken + 1)->symbol == SymbolEnum::RETURNRW) {
        children.push_back(expect(SymbolEnum::RETURNRW));
        if ((m_currToken + 1)->symbol != SymbolEnum::SEMICO) {
            children.push_back(exp(level));
        }
        children.push_back(expect(SymbolEnum::SEMICO));
    } else if ((m_currToken + 1)->symbol == SymbolEnum::PRINTFRW) {
        children.push_back(expect(SymbolEnum::PRINTFRW));
        children.push_back(expect(SymbolEnum::LPARENT));
        children.push_back(expect(SymbolEnum::STRCON));
        while ((m_currToken + 1)->symbol != SymbolEnum::COMMA) {
            children.push_back(expect(SymbolEnum::COMMA));
            children.push_back(exp(level));
        }
        children.push_back(expect(SymbolEnum::RPARENT));
        children.push_back(expect(SymbolEnum::SEMICO));
    }
    // block 的首符 '{'
    else if ((m_currToken + 1)->symbol == SymbolEnum::LBRACE) {
        children.push_back(expect(SymbolEnum::LBRACE));
        children.push_back(block(level + 1));
        children.push_back(expect(SymbolEnum::RBRACE));
    }
    // 处理赋值给左值的情况，包括 lVal '=' exp ';'| lVal '=' 'getint''('')'';'
    else if (expectAssignment()) {
        children.push_back(lVal(level));
        children.push_back(expect(SymbolEnum::ASSIGN));
        if ((m_currToken + 1)->symbol == SymbolEnum::GETINTRW) {
            children.push_back(expect(SymbolEnum::GETINTRW));
            children.push_back(expect(SymbolEnum::LPARENT));
            children.push_back(expect(SymbolEnum::RPARENT));
        } else {
            children.push_back(exp(level));
        }
        children.push_back(expect(SymbolEnum::SEMICO));
    }
    // 处理单一表达式的情况
    else if (expectPureExp()) {
        children.push_back(exp(level));
        children.push_back(expect(SymbolEnum::SEMICO));
    }
    // 只有分号
    else if ((m_currToken + 1)->symbol == SymbolEnum::SEMICO) {
        children.push_back(expect(SymbolEnum::SEMICO));
    }

    for (auto& child : children) {
        stmtNode->addChild(std::move(child));
    }
    return stmtNode;
}

// 左值 lVal -> ident {'[' exp ']'}
std::shared_ptr<VNodeBase> Parser::lVal(int level) {
    std::vector<std::shared_ptr<VNodeBase>> children;
    auto lValNode = std::make_shared<VNodeBranch>(VNodeEnum::LVAL);
    lValNode->setLevel(level);
    children.push_back(expect(SymbolEnum::IDENT));
    int cnt = 0;
    while ((m_currToken + 1)->symbol == SymbolEnum::LBRACK) {
        children.push_back(expect(SymbolEnum::LBRACK));
        children.push_back(exp(level));
        children.push_back(expect(SymbolEnum::RBRACK));
        cnt++;
    }
    if (cnt >= 2) {
        // TODO: handle multi > 2 dimension array
    }
    for (auto& child : children) {
        lValNode->addChild(std::move(child));
    }
    return lValNode;
}
// 表达式 exp -> addExp
std::shared_ptr<VNodeBase> Parser::exp(int level) {
    auto expNode = std::make_shared<VNodeBranch>(VNodeEnum::EXP);
    expNode->setLevel(level);
    expNode->addChild(addExp(level));
    return expNode;
}

// 条件表达式 cond -> lOrExp
std::shared_ptr<VNodeBase> Parser::cond(int level) {
    auto condNode = std::make_shared<VNodeBranch>(VNodeEnum::COND);
    condNode->setLevel(level);
    condNode->addChild(lOrExp(level));
    return condNode;
}

// 基本表达式 primaryExp → '(' exp ')' | lVal | number
std::shared_ptr<VNodeBase> Parser::primaryExp(int level) {
    std::vector<std::shared_ptr<VNodeBase>> children;
    auto primaryExpNode = std::make_shared<VNodeBranch>(VNodeEnum::PRIMARYEXP);
    primaryExpNode->setLevel(level);
    if ((m_currToken + 1)->symbol == SymbolEnum::LPARENT) {
        children.push_back(expect(SymbolEnum::LPARENT));
        children.push_back(exp(level));
        children.push_back(expect(SymbolEnum::RPARENT));
    } else if ((m_currToken + 1)->symbol == SymbolEnum::INTCON) {
        children.push_back(expect(SymbolEnum::INTCON));
    } else {
        children.push_back(lVal(level));
    }
    for (auto& child : children) {
        primaryExpNode->addChild(std::move(child));
    }
    return primaryExpNode;
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