#include <grammar/Parser.h>
#include <Log.h>

Parser::Parser(std::vector<Token>& tokenList) {
    m_tokenList.emplace_back();
    m_tokenList.insert(m_tokenList.end(), tokenList.begin(), tokenList.end());
    m_currToken = m_tokenList.begin();
}

void Parser::parse() {
    m_astRoot = compUnit(0);
    m_currToken++;
    if (m_currToken != m_tokenList.end()) {
        Logger::logInfo("Token解析未完成");
    }
}

std::shared_ptr<VNodeBase> Parser::getASTRoot() const {
    return m_astRoot;
}

void Parser::traversalAST(std::filebuf& file) {
    std::ostream os(&file);
    postTraversal(m_astRoot, os);
    //preTraversal(m_astRoot, os);
}

void Parser::postTraversal(std::shared_ptr<VNodeBase> node, std::ostream& os) {
    if (node->getType() == VType::VN) {
        auto branch = std::static_pointer_cast<VNodeBranch>(node);
        auto children = branch->getChildren();
        for (auto& child : children) {
            postTraversal(child, os);
        }
        if (!(branch->getNodeEnum() == VNodeEnum::DECL
              || branch->getNodeEnum() == VNodeEnum::BLOCKITEM
              || branch->getNodeEnum() == VNodeEnum::BTYPE)) {
            branch->dumpToFile(os);
        }
    } else {
        auto leaf = std::static_pointer_cast<VNodeLeaf>(node);
        leaf->dumpToFile(os);
    }
}

void Parser::preTraversal(std::shared_ptr<VNodeBase> node, std::ostream& os) {
    if (node->getType() == VType::VN) {
        auto branch = std::static_pointer_cast<VNodeBranch>(node);
        branch->dumpToFile(os);
        auto children = branch->getChildren();
        for (auto& child : children) {
            postTraversal(child, os);
        }
    } else {
        auto leaf = std::static_pointer_cast<VNodeLeaf>(node);
        leaf->dumpToFile(os);
    }
}

// 期望获取symbol对应的内容，并返回生成的叶节点
std::shared_ptr<VNodeBase> Parser::expect(SymbolEnum symbol, int level) {
    if ((m_currToken + 1)->symbol == symbol) {
        m_currToken++;
        auto leaf = std::make_shared<VNodeLeaf>(symbol, *m_currToken);
        leaf->setLevel(level);
        return leaf;
    } else {
        std::string literal;
        if (!m_probingMode) {
            literal = handleGrammarError(symbol);
        }
        return std::make_shared<VNodeLeaf>(symbol, Token(m_currToken->lineNum, symbol, literal, 0), m_probingMode ? true : false); // 如果全是false会导致死循环，这么写会导致expect生成式的函数出错
    }
}

// 期望获取symbolList包含的内容，并返回生成的叶节点
std::shared_ptr<VNodeBase> Parser::expect(std::initializer_list<SymbolEnum> symbolList, int level) {
    std::set<SymbolEnum> symset(symbolList);
    if (symset.count((m_currToken + 1)->symbol)) {
        m_currToken++;
        auto leaf = std::make_shared<VNodeLeaf>(m_currToken->symbol, *m_currToken);
        leaf->setLevel(level);
        return leaf;
    } else {
        std::string literal;
        if (!m_probingMode) {
            literal = handleGrammarError(m_currToken->symbol);
        }
        return std::make_shared<VNodeLeaf>(*symbolList.begin(),
                                           Token(m_currToken->lineNum, *symbolList.begin(), literal, 0), m_probingMode ? true : false);
    }
}

std::string Parser::handleGrammarError(SymbolEnum symbol) {
    switch (symbol) {
    case SymbolEnum::SEMICN:
        Logger::logError(ErrorType::MISSING_SEMICN, m_currToken->lineNum);
        return ";";
    case SymbolEnum::RPARENT:
        Logger::logError(ErrorType::MISSING_RPARENT, m_currToken->lineNum);
        return ")";
    case SymbolEnum::RBRACK:
        Logger::logError(ErrorType::MISSING_RBRACK, m_currToken->lineNum);
        return "]";
    default: break;
    }
    return "";
}

bool Parser::expectAssignment() {
    auto iterBefore = m_currToken;
    m_probingMode = true;
    auto temp = lVal(-1);
    if (temp->isCorrect()) {
        bool correct = (m_currToken + 1)->symbol == SymbolEnum::ASSIGN;
        m_currToken = iterBefore;
        m_probingMode = false;
        return correct;
    } else {
        m_currToken = iterBefore;
        m_probingMode = false;
        return false;
    }
}
bool Parser::expectPureExp() {
    auto iterBefore = m_currToken;
    m_probingMode = true;
    auto temp = exp(-1);
    m_probingMode = false;
    if (temp->isCorrect() && m_currToken != iterBefore) {
        m_currToken = iterBefore;
        return true;
    } else {
        m_currToken = iterBefore;
        return false;
    }
}

bool Parser::expectUnaryOp() {
    auto nextIter = m_currToken + 1;
    return nextIter->symbol == SymbolEnum::PLUS
           || nextIter->symbol == SymbolEnum::MINU
           || nextIter->symbol == SymbolEnum::NOT;
}

bool Parser::expectFuncRParams() {
    auto iterBefore = m_currToken;
    m_probingMode = true;
    funcRParams(-1);
    m_probingMode = false;
    if (m_currToken == iterBefore) {
        return false;
    } else {
        m_currToken = iterBefore;
        return true;
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
    while ((m_currToken + 2)->symbol != SymbolEnum::MAINTK) {
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
    if ((m_currToken + 1)->symbol == SymbolEnum::CONSTTK) {
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
    children.push_back(expect(SymbolEnum::CONSTTK, level));
    children.push_back(bType(level));
    children.push_back(constDef(level));
    while ((m_currToken + 1)->symbol == SymbolEnum::COMMA) {
        children.push_back(expect(SymbolEnum::COMMA, level));
        children.push_back(constDef(level));
    }
    children.push_back(expect(SymbolEnum::SEMICN, level));
    for (auto& child : children) {
        constDeclNode->addChild(std::move(child));
    }
    return constDeclNode;
}

// 基本类型bType -> 'int'
std::shared_ptr<VNodeBase> Parser::bType(int level) {
    auto btypeNode = std::make_shared<VNodeBranch>(VNodeEnum::BTYPE);
    btypeNode->setLevel(level);
    if ((m_currToken + 1)->symbol == SymbolEnum::INTTK) {
        btypeNode->addChild(expect(SymbolEnum::INTTK, level));
    } else {
        btypeNode->addChild(expect(SymbolEnum::CHARTK, level));
    }

    return btypeNode;
}

// 常量定义constDef -> IDENFR {'[' constExp ']'} '=' constInitVal;
std::shared_ptr<VNodeBase> Parser::constDef(int level) {
    std::vector<std::shared_ptr<VNodeBase>> children;
    auto constDefNode = std::make_shared<VNodeBranch>(VNodeEnum::CONSTDEF);
    constDefNode->setLevel(level);
    children.push_back(expect(SymbolEnum::IDENFR, level));
    while ((m_currToken + 1)->symbol == SymbolEnum::LBRACK) {
        children.push_back(expect(SymbolEnum::LBRACK, level));
        children.push_back(constExp(level));
        children.push_back(expect(SymbolEnum::RBRACK, level));
    }
    children.push_back(expect(SymbolEnum::ASSIGN, level));
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
        children.push_back(expect(SymbolEnum::LBRACE, level));
        if ((m_currToken + 1)->symbol != SymbolEnum::RBRACE) {
            children.push_back(constInitVal(level));
            while ((m_currToken + 1)->symbol == SymbolEnum::COMMA) {
                children.push_back(expect(SymbolEnum::COMMA, level));
                children.push_back(constInitVal(level));
            }
        }
        children.push_back(expect(SymbolEnum::RBRACE, level));
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

// 变量声明 varDecl -> bType varDef { ',' varDef } ';'
std::shared_ptr<VNodeBase> Parser::varDecl(int level) {
    std::vector<std::shared_ptr<VNodeBase>> children;
    auto varDeclNode = std::make_shared<VNodeBranch>(VNodeEnum::VARDECL);
    varDeclNode->setLevel(level);
    children.push_back(bType(level));
    children.push_back(varDef(level));
    while ((m_currToken + 1)->symbol == SymbolEnum::COMMA) {
        children.push_back(expect(SymbolEnum::COMMA, level));
        children.push_back(varDef(level));
    }
    children.push_back(expect(SymbolEnum::SEMICN, level));
    for (auto& child : children) {
        varDeclNode->addChild(std::move(child));
    }
    return varDeclNode;
}
// 变量定义 varDef -> IDENFR { '[' constExp ']' } [ '=' initVal ]
std::shared_ptr<VNodeBase> Parser::varDef(int level) {
    std::vector<std::shared_ptr<VNodeBase>> children;
    auto varDefNode = std::make_shared<VNodeBranch>(VNodeEnum::VARDEF);
    varDefNode->setLevel(level);
    children.push_back(expect(SymbolEnum::IDENFR, level));
    while ((m_currToken + 1)->symbol == SymbolEnum::LBRACK) {
        children.push_back(expect(SymbolEnum::LBRACK, level));
        children.push_back(constExp(level));
        children.push_back(expect(SymbolEnum::RBRACK, level));
    }
    if ((m_currToken + 1)->symbol == SymbolEnum::ASSIGN) {
        children.push_back(expect(SymbolEnum::ASSIGN, level));
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
        children.push_back(expect(SymbolEnum::LBRACE, level));
        if ((m_currToken + 1)->symbol != SymbolEnum::RBRACE) {
            children.push_back(initVal(level));
            while ((m_currToken + 1)->symbol == SymbolEnum::COMMA) {
                children.push_back(expect(SymbolEnum::COMMA, level));
                children.push_back(initVal(level));
            }
        }
        children.push_back(expect(SymbolEnum::RBRACE, level));
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
    children.push_back(expect(SymbolEnum::LBRACE, level));
    while ((m_currToken + 1)->symbol != SymbolEnum::RBRACE) {
        children.push_back(blockItem(level + 1));
    }
    children.push_back(expect(SymbolEnum::RBRACE, level));
    for (auto& child : children) {
        blockNode->addChild(std::move(child));
    }
    return blockNode;
}

// 语句块项 blockItem -> decl | stmt
std::shared_ptr<VNodeBase> Parser::blockItem(int level) {
    auto blockItemNode = std::make_shared<VNodeBranch>(VNodeEnum::BLOCKITEM);
    blockItemNode->setLevel(level);
    if ((m_currToken + 1)->symbol == SymbolEnum::CONSTTK
        || (m_currToken + 1)->symbol == SymbolEnum::INTTK
        || (m_currToken + 1)->symbol == SymbolEnum::CHARTK) {
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
    if ((m_currToken + 1)->symbol == SymbolEnum::IFTK) {
        children.push_back(expect(SymbolEnum::IFTK, level));
        children.push_back(expect(SymbolEnum::LPARENT, level));
        children.push_back(cond(level));
        children.push_back(expect(SymbolEnum::RPARENT, level));
        children.push_back(stmt(level));
        if ((m_currToken + 1)->symbol == SymbolEnum::ELSETK) {
            children.push_back(expect(SymbolEnum::ELSETK, level));
            children.push_back(stmt(level));
        }
    } else if ((m_currToken + 1)->symbol == SymbolEnum::WHILETK) {
        children.push_back(expect(SymbolEnum::WHILETK, level));
        children.push_back(expect(SymbolEnum::LPARENT, level));
        children.push_back(cond(level));
        children.push_back(expect(SymbolEnum::RPARENT, level));
        children.push_back(stmt(level));
    } else if ((m_currToken + 1)->symbol == SymbolEnum::BREAKTK || (m_currToken + 1)->symbol == SymbolEnum::CONTINUETK) {
        children.push_back(expect({SymbolEnum::BREAKTK, SymbolEnum::CONTINUETK}, level));
        children.push_back(expect(SymbolEnum::SEMICN, level));
    } else if ((m_currToken + 1)->symbol == SymbolEnum::RETURNTK) {
        children.push_back(expect(SymbolEnum::RETURNTK, level));
        if ((m_currToken + 1)->symbol != SymbolEnum::SEMICN
            && (m_currToken + 1)->symbol != SymbolEnum::RBRACE) {
            children.push_back(exp(level));
        }
        children.push_back(expect(SymbolEnum::SEMICN, level));
    } else if ((m_currToken + 1)->symbol == SymbolEnum::PRINTFTK) {
        children.push_back(expect(SymbolEnum::PRINTFTK, level));
        children.push_back(expect(SymbolEnum::LPARENT, level));
        children.push_back(expect(SymbolEnum::STRCON, level));
        while ((m_currToken + 1)->symbol == SymbolEnum::COMMA) {
            children.push_back(expect(SymbolEnum::COMMA, level));
            children.push_back(exp(level));
        }
        children.push_back(expect(SymbolEnum::RPARENT, level));
        children.push_back(expect(SymbolEnum::SEMICN, level));
    }
    // block 的首符 '{'
    else if ((m_currToken + 1)->symbol == SymbolEnum::LBRACE) {
        children.push_back(block(level + 1));
    }
    // 处理赋值给左值的情况，包括 lVal '=' exp ';'| lVal '=' 'getint''('')'';'
    else if (expectAssignment()) {
        children.push_back(lVal(level));
        children.push_back(expect(SymbolEnum::ASSIGN, level));
        if ((m_currToken + 1)->symbol == SymbolEnum::GETINTTK) {
            children.push_back(expect(SymbolEnum::GETINTTK, level));
            children.push_back(expect(SymbolEnum::LPARENT, level));
            children.push_back(expect(SymbolEnum::RPARENT, level));
        } else {
            children.push_back(exp(level));
        }
        children.push_back(expect(SymbolEnum::SEMICN, level));
    }
    // 处理单一表达式的情况
    else if (expectPureExp()) {
        children.push_back(exp(level));
        children.push_back(expect(SymbolEnum::SEMICN, level));
    }
    // 只有分号
    else if ((m_currToken + 1)->symbol == SymbolEnum::SEMICN) {
        children.push_back(expect(SymbolEnum::SEMICN, level));
    }

    for (auto& child : children) {
        stmtNode->addChild(std::move(child));
    }
    return stmtNode;
}

// 左值 lVal -> IDENFR {'[' exp ']'}
std::shared_ptr<VNodeBase> Parser::lVal(int level) {
    std::vector<std::shared_ptr<VNodeBase>> children;
    auto lValNode = std::make_shared<VNodeBranch>(VNodeEnum::LVAL);
    lValNode->setLevel(level);
    children.push_back(expect(SymbolEnum::IDENFR, level));
    int cnt = 0;
    while ((m_currToken + 1)->symbol == SymbolEnum::LBRACK) {
        children.push_back(expect(SymbolEnum::LBRACK, level));
        children.push_back(exp(level));
        children.push_back(expect(SymbolEnum::RBRACK, level));
        cnt++;
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

std::shared_ptr<VNodeBase> Parser::number(int level) {
    auto numberNode = std::make_shared<VNodeBranch>(VNodeEnum::NUM);
    numberNode->setLevel(level);
    numberNode->addChild(expect(SymbolEnum::INTCON, level));
    return numberNode;
}

// 基本表达式 primaryExp -> '(' exp ')' | lVal | number
std::shared_ptr<VNodeBase> Parser::primaryExp(int level) {
    std::vector<std::shared_ptr<VNodeBase>> children;
    auto primaryExpNode = std::make_shared<VNodeBranch>(VNodeEnum::PRIMARYEXP);
    primaryExpNode->setLevel(level);
    if ((m_currToken + 1)->symbol == SymbolEnum::LPARENT) {
        children.push_back(expect(SymbolEnum::LPARENT, level));
        children.push_back(exp(level));
        children.push_back(expect(SymbolEnum::RPARENT, level));
    } else if ((m_currToken + 1)->symbol == SymbolEnum::INTCON) {
        children.push_back(number(level));
    } else {
        children.push_back(lVal(level));
    }
    for (auto& child : children) {
        primaryExpNode->addChild(std::move(child));
    }
    return primaryExpNode;
}

// 一元表达式 unaryExp -> primaryExp | IDENFR '(' [funcRParams] ')' | | unaryOp unaryExp
std::shared_ptr<VNodeBase> Parser::unaryExp(int level) {
    std::vector<std::shared_ptr<VNodeBase>> children;
    auto unaryExpNode = std::make_shared<VNodeBranch>(VNodeEnum::UNARYEXP);
    unaryExpNode->setLevel(level);
    if ((m_currToken + 1)->symbol == SymbolEnum::IDENFR
        && (m_currToken + 2)->symbol == SymbolEnum::LPARENT) {
        children.push_back(expect(SymbolEnum::IDENFR, level));
        children.push_back(expect(SymbolEnum::LPARENT, level));
        // TODO: 修复'('错误
        if ((m_currToken + 1)->symbol != SymbolEnum::RPARENT
            && (m_currToken + 1)->symbol != SymbolEnum::SEMICN) {
            children.push_back(funcRParams(level));
        }
        children.push_back(expect(SymbolEnum::RPARENT, level));

    } else if (expectUnaryOp()) {
        children.push_back(unaryOp(level));
        children.push_back(unaryExp(level));
    } else {
        children.push_back(primaryExp(level));
    }

    for (auto& child : children) {
        unaryExpNode->addChild(std::move(child));
    }
    return unaryExpNode;
}

// 单目运算符 unaryOp -> '+' | '−' | '!'
std::shared_ptr<VNodeBase> Parser::unaryOp(int level) {
    auto unaryOpNode = std::make_shared<VNodeBranch>(VNodeEnum::UNARYOP);
    unaryOpNode->setLevel(level);
    auto op = expect({SymbolEnum::PLUS, SymbolEnum::MINU, SymbolEnum::NOT}, level);
    unaryOpNode->addChild(std::move(op));
    return unaryOpNode;
}

// 加减模运算 addExp -> mulExp {('+' | '−') mulExp}
std::shared_ptr<VNodeBase> Parser::addExp(int level) {
    auto addExpNode = std::make_shared<VNodeBranch>(VNodeEnum::ADDEXP);
    addExpNode->setLevel(level);
    addExpNode->addChild(mulExp(level));
    while ((m_currToken + 1)->symbol == SymbolEnum::PLUS || (m_currToken + 1)->symbol == SymbolEnum::MINU) {
        auto newAddExpNode = std::make_shared<VNodeBranch>(VNodeEnum::ADDEXP);
        newAddExpNode->setLevel(level);
        newAddExpNode->addChild(addExpNode);
        addExpNode = newAddExpNode;
        addExpNode->addChild(expect({SymbolEnum::PLUS, SymbolEnum::MINU}, level));
        addExpNode->addChild(mulExp(level));
    }
    return addExpNode;
}

// 乘除模运算 mulExp -> unaryExp { ('*' | '/' | '%') unaryExp}
std::shared_ptr<VNodeBase> Parser::mulExp(int level) {
    auto mulExpNode = std::make_shared<VNodeBranch>(VNodeEnum::MULEXP);
    mulExpNode->setLevel(level);
    mulExpNode->addChild(unaryExp(level));
    while ((m_currToken + 1)->symbol == SymbolEnum::MULT
           || (m_currToken + 1)->symbol == SymbolEnum::DIV
           || (m_currToken + 1)->symbol == SymbolEnum::MOD) {
        auto newMulExpNode = std::make_shared<VNodeBranch>(VNodeEnum::MULEXP);
        newMulExpNode->setLevel(level);
        newMulExpNode->addChild(mulExpNode);
        mulExpNode = newMulExpNode;
        mulExpNode->addChild(expect({SymbolEnum::MULT, SymbolEnum::DIV, SymbolEnum::MOD}, level));
        mulExpNode->addChild(unaryExp(level));
    }
    return mulExpNode;
}

// 关系表达式 relExp -> addExp {('<' | '>' | '<=' | '>=') addExp}
std::shared_ptr<VNodeBase> Parser::relExp(int level) {
    auto relExpNode = std::make_shared<VNodeBranch>(VNodeEnum::RELEXP);
    relExpNode->setLevel(level);
    relExpNode->addChild(addExp(level));
    while ((m_currToken + 1)->symbol == SymbolEnum::LSS
           || (m_currToken + 1)->symbol == SymbolEnum::LEQ
           || (m_currToken + 1)->symbol == SymbolEnum::GRE
           || (m_currToken + 1)->symbol == SymbolEnum::GEQ) {
        auto newRelExpNode = std::make_shared<VNodeBranch>(VNodeEnum::RELEXP);
        newRelExpNode->setLevel(level);
        newRelExpNode->addChild(relExpNode);
        relExpNode = newRelExpNode;
        relExpNode->addChild(expect({SymbolEnum::LSS, SymbolEnum::LEQ, SymbolEnum::GRE, SymbolEnum::GEQ}, level));
        relExpNode->addChild(addExp(level));
    }
    return relExpNode;
}

// 相等性表达式 eqExp -> relExp { ('==' | '!=') relExp}
std::shared_ptr<VNodeBase> Parser::eqExp(int level) {
    auto eqExpNode = std::make_shared<VNodeBranch>(VNodeEnum::EQEXP);
    eqExpNode->setLevel(level);
    eqExpNode->addChild(relExp(level));
    while ((m_currToken + 1)->symbol == SymbolEnum::EQL || (m_currToken + 1)->symbol == SymbolEnum::NEQ) {
        auto newEqExpNode = std::make_shared<VNodeBranch>(VNodeEnum::EQEXP);
        newEqExpNode->addChild(eqExpNode);
        newEqExpNode->setLevel(level);
        eqExpNode = newEqExpNode;
        eqExpNode->addChild(expect({SymbolEnum::EQL, SymbolEnum::NEQ}, level));
        eqExpNode->addChild(relExp(level));
    }
    return eqExpNode;
}

// 逻辑与表达式 lAndExp -> eqExp { '&&' eqExp }
std::shared_ptr<VNodeBase> Parser::lAndExp(int level) {
    auto lAndExpNode = std::make_shared<VNodeBranch>(VNodeEnum::LANDEXP);
    lAndExpNode->setLevel(level);
    lAndExpNode->addChild(eqExp(level));
    while ((m_currToken + 1)->symbol == SymbolEnum::AND) {
        auto newLAndExpNode = std::make_shared<VNodeBranch>(VNodeEnum::LANDEXP);
        newLAndExpNode->setLevel(level);
        newLAndExpNode->addChild(lAndExpNode);
        lAndExpNode = newLAndExpNode;
        lAndExpNode->addChild(expect(SymbolEnum::AND, level));
        lAndExpNode->addChild(eqExp(level));
    }
    return lAndExpNode;
}

// 逻辑或表达式 lOrExp ->  lAndExp { '||' lAndExp }
std::shared_ptr<VNodeBase> Parser::lOrExp(int level) {
    auto lOrExpNode = std::make_shared<VNodeBranch>(VNodeEnum::LOREXP);
    lOrExpNode->setLevel(level);
    lOrExpNode->addChild(lAndExp(level));
    while ((m_currToken + 1)->symbol == SymbolEnum::OR) {
        auto newLOrExpNode = std::make_shared<VNodeBranch>(VNodeEnum::LOREXP);
        newLOrExpNode->setLevel(level);
        newLOrExpNode->addChild(lOrExpNode);
        lOrExpNode = newLOrExpNode;
        lOrExpNode->addChild(expect(SymbolEnum::OR, level));
        lOrExpNode->addChild(lAndExp(level));
    }
    return lOrExpNode;
}

// 函数定义 funcDef -> funcType IDENFR '(' [funcFParams] ')' block
std::shared_ptr<VNodeBase> Parser::funcDef(int level) {
    std::vector<std::shared_ptr<VNodeBase>> children;
    auto funcDefNode = std::make_shared<VNodeBranch>(VNodeEnum::FUNCDEF);
    funcDefNode->setLevel(level);
    children.push_back(funcType(level));
    children.push_back(expect(SymbolEnum::IDENFR, level));
    children.push_back(expect(SymbolEnum::LPARENT, level));
    if ((m_currToken + 1)->symbol != SymbolEnum::RPARENT
        && (m_currToken + 1)->symbol != SymbolEnum::LBRACE) {
        children.push_back(funcFParams(level));
    }
    children.push_back(expect(SymbolEnum::RPARENT, level));
    children.push_back(block(level + 1));

    for (auto& child : children) {
        funcDefNode->addChild(std::move(child));
    }
    return funcDefNode;
}

// 主函数定义 mainFuncDef -> 'int' 'main' '(' ')' block
std::shared_ptr<VNodeBase> Parser::mainFuncDef(int level) {
    std::vector<std::shared_ptr<VNodeBase>> children;
    auto mainFuncDefNode = std::make_shared<VNodeBranch>(VNodeEnum::MAINFUNCDEF);
    mainFuncDefNode->setLevel(level);
    children.push_back(expect(SymbolEnum::INTTK, level));
    children.push_back(expect(SymbolEnum::MAINTK, level));
    children.push_back(expect(SymbolEnum::LPARENT, level));
    children.push_back(expect(SymbolEnum::RPARENT, level));
    children.push_back(block(level + 1));
    for (auto& child : children) {
        mainFuncDefNode->addChild(std::move(child));
    }
    return mainFuncDefNode;
}

// 函数类型 funcType -> 'void' | 'int'
std::shared_ptr<VNodeBase> Parser::funcType(int level) {
    auto funcTypeNode = std::make_shared<VNodeBranch>(VNodeEnum::FUNCTYPE);
    funcTypeNode->setLevel(level);
    funcTypeNode->addChild(expect({SymbolEnum::VOIDTK, SymbolEnum::INTTK, SymbolEnum::CHARTK}, level));
    return funcTypeNode;
}

// 函数形参表 funcFParams -> funcFParam {',' funcFParam}
std::shared_ptr<VNodeBase> Parser::funcFParams(int level) {
    std::vector<std::shared_ptr<VNodeBase>> children;
    auto funcFParamsNode = std::make_shared<VNodeBranch>(VNodeEnum::FUNCFPARAMS);
    funcFParamsNode->setLevel(level);
    children.push_back(funcFParam(level));
    while ((m_currToken + 1)->symbol == SymbolEnum::COMMA) {
        children.push_back(expect(SymbolEnum::COMMA, level));
        children.push_back(funcFParam(level));
    }

    for (auto& child : children) {
        funcFParamsNode->addChild(std::move(child));
    }
    return funcFParamsNode;
}

// 函数形参 funcFParam -> bType IDENFR ['[' ']' { '[' constExp ']' }]
std::shared_ptr<VNodeBase> Parser::funcFParam(int level) {
    std::vector<std::shared_ptr<VNodeBase>> children;
    auto funcFParamNode = std::make_shared<VNodeBranch>(VNodeEnum::FUNCFPARAM);
    funcFParamNode->setLevel(level);
    children.push_back(bType(level));
    children.push_back(expect(SymbolEnum::IDENFR, level));
    if ((m_currToken + 1)->symbol == SymbolEnum::LBRACK) {
        children.push_back(expect(SymbolEnum::LBRACK, level));
        children.push_back(expect(SymbolEnum::RBRACK, level));
        while ((m_currToken + 1)->symbol == SymbolEnum::LBRACK) {
            children.push_back(expect(SymbolEnum::LBRACK, level));
            children.push_back(constExp(level));
            children.push_back(expect(SymbolEnum::RBRACK, level));
        }
    }

    for (auto& child : children) {
        funcFParamNode->addChild(std::move(child));
    }
    return funcFParamNode;
}

// 函数实参表 funcRParams -> exp { ',' exp }
std::shared_ptr<VNodeBase> Parser::funcRParams(int level) {
    std::vector<std::shared_ptr<VNodeBase>> children;
    auto funcRParamsNode = std::make_shared<VNodeBranch>(VNodeEnum::FUNCRPARAMS);

    funcRParamsNode->setLevel(level);
    children.push_back(exp(level));
    while ((m_currToken + 1)->symbol == SymbolEnum::COMMA) {
        children.push_back(expect(SymbolEnum::COMMA, level));
        children.push_back(exp(level));
    }
    for (auto& child : children) {
        funcRParamsNode->addChild(std::move(child));
    }
    return funcRParamsNode;
}
