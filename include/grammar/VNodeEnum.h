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
    FUNCRPARAMS,
};
static const std::vector<std::string> s_vnodeEnumText{
    "COMPUNIT",
    "DECL",
    "CONSTDECL",
    "BTYPE",
    "CONSTDEF",
    "CONSTINITVAL",
    "CONSTEXP",
    "VARDECL",
    "VARDEF",
    "INITVAL",
    "BLOCK",
    "BLOCKITEM",
    "STMT",
    "LVAL",
    "EXP",
    "COND",
    "PRIMARYEXP",
    "UNARYEXP",
    "UNARYOP",
    "ADDEXP",
    "MULEXP",
    "RELEXP",
    "EQEXP",
    "LANDEXP",
    "LOREXP",
    "FUNCDEF",
    "MAINFUNCDEF",
    "FUNCTYPE",
    "FUNCFPARAMS",
    "FUNCRPARAMS",
};

static const std::string& getVNodeEnumText(VNodeEnum nodeEnum) {
    return s_vnodeEnumText[static_cast<std::underlying_type<VNodeEnum>::type>(nodeEnum)];
}
#endif