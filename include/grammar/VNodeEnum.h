#ifndef VN_TERM_ENUM_H
#define VN_TERM_ENUM_H
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
#endif