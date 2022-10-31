#include <symbol/SymbolTableItem.h>
#include <symbol/ValueType.h>
template <>
size_t VarItem<ArrayType<IntType>>::getSize() {
    return m_varItem.spaceSize(TypedItem<ArrayType<IntType>>::m_type.valueSize());
};

template <>
size_t ConstVarItem<ArrayType<IntType>>::getSize() {
    return m_constVar.spaceSize(TypedItem<ArrayType<IntType>>::m_type.valueSize());
};