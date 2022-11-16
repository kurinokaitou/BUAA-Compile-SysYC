#ifndef MIPS_REG_ENUM_H
#define MIPS_REG_ENUM_H
#include <map>
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

static std::map<MipsReg, std::string> s_realRegMap{
    {MipsReg::zero, "$zero"},
    {MipsReg::at, "$at"},
    // return val
    {MipsReg::v0, "$v0"},
    {MipsReg::v1, "$v1"},
    // params
    {MipsReg::a0, "$a0"},
    {MipsReg::a1, "$a1"},
    {MipsReg::a2, "$a2"},
    {MipsReg::a3, "$a3"},
    // temp
    {MipsReg::t0, "$t0"},
    {MipsReg::t1, "$t1"},
    {MipsReg::t2, "$t2"},
    {MipsReg::t3, "$t3"},
    {MipsReg::t4, "$t4"},
    {MipsReg::t5, "$t5"},
    {MipsReg::t6, "$t6"},
    {MipsReg::t7, "$t7"},
    // saved
    {MipsReg::s0, "$s0"},
    {MipsReg::s1, "$s1"},
    {MipsReg::s2, "$s2"},
    {MipsReg::s3, "$s3"},
    {MipsReg::s4, "$s4"},
    {MipsReg::s5, "$s5"},
    {MipsReg::s6, "$s6"},
    {MipsReg::s7, "$s7"},
    // temp(addr)
    {MipsReg::t8, "$t8"},
    {MipsReg::t9, "$t9"},
    // exception
    {MipsReg::k0, "$k0"},
    {MipsReg::k1, "$k1"},
    // special
    {MipsReg::gp, "$gp"}, // global pointer
    {MipsReg::sp, "$sp"}, // stack pointer
    {MipsReg::fp, "$fp"}, // frame pointer
    {MipsReg::ra, "$ra"}};
#endif