#ifndef MIPS_REG_ENUM_H
#define MIPS_REG_ENUM_H
enum class MipsReg : int {
    // zero & at
    zero = 0,
    at,
    // return val
    v0,
    v1,
    // params
    a0,
    a1,
    a2,
    a3,
    // temp
    t0,
    t1,
    t2,
    t3,
    t4,
    t5,
    t6,
    t7,
    // saved
    s0,
    s1,
    s2,
    s3,
    s4,
    s5,
    s6,
    s7,
    // temp(addr)
    t8,
    t9,
    // exception
    k0,
    k1,
    // special
    gp, // global pointer
    sp, // stack pointer
    fp, // frame pointer
    ra  // return addr
};
#endif