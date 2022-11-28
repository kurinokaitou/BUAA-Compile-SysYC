#ifndef ALLOCATE_REGISTER_H
#define ALLOCATE_REGISTER_H
class MipsModule;
void allocateRegister(MipsModule& module);
void peepholeOpt(MipsModule& module);
void computeStackInfo(MipsModule& module);
#endif