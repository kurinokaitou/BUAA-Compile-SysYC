#ifndef SYMBOL_TABLE_ITEM_H
#define SYMBOL_TABLE_ITEM_H
#include "BlockScopeHandle.h"
#include "ValueType.h"

class SymbolTableItem {
public:
    SymbolTableItem(const std::string& name, BlockScopeHandle handle) :
        m_name(name), m_scopeHandle(handle){};
    virtual ~SymbolTableItem() {}
    const std::string& getName() { return m_name; }
    bool isParam() const { return m_isParam; }
    void setParam() { m_isParam = true; }
    void setLevel(int level) { m_level = level; }
    int getLevel() const { return m_level; }
    BlockScopeHandle getParentHandle() const { return m_scopeHandle; }

private:
    std::string m_name;
    BlockScopeHandle m_scopeHandle;
    bool m_isParam{false};
    int m_level{0};
};

// 以下所有的Type均为ValueType类型的子类
template <typename Type>
class TypedItem : public SymbolTableItem {
public:
    explicit TypedItem(const std::string& name, BlockScopeHandle handle) :
        SymbolTableItem(name, handle) {}
    virtual ~TypedItem() {}
    Type& getType() { return m_dataType; }

protected:
    Type m_dataType;
};

template <typename Type>
static Type& getValueType(SymbolTableItem* item) {
    return dynamic_cast<TypedItem<Type>*>(item)->getType();
}

template <typename Type>
class FuncItem : public TypedItem<Type> {
public:
    using Data = struct {
        BlockScopeHandle parentHandle;
        std::vector<SymbolTableItem*> params;
    };
    explicit FuncItem(const std::string& name, Data data) :
        TypedItem<Type>(name, data.parentHandle), m_params(data.params){};
    std::vector<SymbolTableItem*>& getParams() { return m_params; }
    virtual ~FuncItem() {}

private:
    std::vector<SymbolTableItem*> m_params;
};

template <typename Type>
class VarItem : public TypedItem<Type> {
public:
    using Data = struct {
        BlockScopeHandle parentHandle;
        typename Type::InternalType var;
    };
    explicit VarItem(const std::string& name, Data data) :
        TypedItem<Type>(name, data.parentHandle), m_var(data.var){};
    virtual ~VarItem() {}
    typename Type::InternalType getVar() const { return m_var; }

private:
    typename Type::InternalType m_var;
};

template <typename Type>
class ConstVarItem : public TypedItem<Type> {
public:
    using Data = struct {
        BlockScopeHandle parentHandle;
        typename Type::InternalType constVar;
    };
    explicit ConstVarItem(const std::string& name, Data data) :
        TypedItem<Type>(name, data.parentHandle), m_constVar(data.constVar){};
    virtual ~ConstVarItem() {}
    typename Type::InternalType getConstVar() const { return m_constVar; }

private:
    typename Type::InternalType m_constVar;
};

#endif