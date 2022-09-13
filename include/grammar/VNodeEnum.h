#ifndef VN_TERM_ENUM_H
#define VN_TERM_ENUM_H
#include <vector>
#include <string>
enum class VType : bool {
    VN = false,
    VT = true
};
enum class VNodeEnum : int {
    COMPUNIT = 0,
    DECL,
    CONSTDECL,
    BTYPE,
    CONSTDEF,
    CONSTINITVAL,
    CONSTEXP,
    VARDECL,
    VARDEF,
    INITVAL,
    BLOCK,
    BLOCKITEM,
    STMT,
    LVAL,
    EXP,
    COND,
    NUM,
    PRIMARYEXP,
    UNARYEXP,
    UNARYOP,
    ADDEXP,
    MULEXP,
    RELEXP,
    EQEXP,
    LANDEXP,
    LOREXP,
    FUNCDEF,
    MAINFUNCDEF,
    FUNCTYPE,
    FUNCFPARAMS,
    FUNCFPARAM,
    FUNCRPARAMS,
};
static const std::vector<std::string> s_vnodeEnumText{
    "CompUnit",
    "Decl",
    "ConstDecl",
    "BType",
    "ConstDef",
    "ConstInitVal",
    "ConstExp",
    "VarDecl",
    "VarDef",
    "InitVal",
    "Block",
    "BlockItem",
    "Stmt",
    "LVal",
    "Exp",
    "Cond",
    "Number",
    "PrimaryExp",
    "UnaryExp",
    "UnaryOp",
    "AddExp",
    "MulExp",
    "RelExp",
    "EqExp",
    "LAndExp",
    "LOrExp",
    "FuncDef",
    "MainFuncDef",
    "FuncType",
    "FuncFParams",
    "FuncFParam",
    "FuncRParams",
};

static const std::string& getVNodeEnumText(VNodeEnum nodeEnum) {
    return s_vnodeEnumText[static_cast<std::underlying_type<VNodeEnum>::type>(nodeEnum)];
}
#endif