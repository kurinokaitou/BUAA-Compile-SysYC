#ifndef SYMBOL_TABLE_ITEM_H
#define SYMBOL_TABLE_ITEM_H
#include "BlockScopeHandle.h"
#include <vector>
#include <memory>
#include <iostream>
#include <algorithm>

enum class ValueTypeEnum : unsigned int {
    INT_TYPE = 0,
    CHAR_TYPE,
    VOID_TYPE,
};
class ValueType;

class Value; // ir的基本单位
class SymbolTableItem {
public:
    SymbolTableItem(const std::string& name) :
        m_name(name){};
    virtual ~SymbolTableItem() {}
    virtual void dumpSymbolItem(std::ostream& os, bool hasType = true) {
        os << m_name;
    };
    virtual ValueType* getType() = 0;
    virtual bool isChangble() = 0;
    virtual bool hasInit() const { return false; }
    const std::string& getName() { return m_name; }
    bool isParam() const { return m_isParam; }
    void setParam() { m_isParam = true; }
    void setLevel(int level) { m_level = level; }
    void setScopeHandle(BlockScopeHandle handle) { m_scopeHandle = handle; }
    int getLevel() const { return m_level; }
    BlockScopeHandle getParentHandle() const { return m_scopeHandle; }
    Value* getIrValue() { return m_irValue; }
    void setIrValue(Value* value) { m_irValue = value; }

private:
    std::string m_name;
    BlockScopeHandle m_scopeHandle;
    Value* m_irValue{nullptr};
    bool m_isParam{false};
    int m_level{0};
};

// 以下所有的Type均为ValueType类型的子类
template <typename Type>
class TypedItem : public SymbolTableItem {
public:
    explicit TypedItem(const std::string& name) :
        SymbolTableItem(name) {}
    virtual ~TypedItem() {}
    virtual ValueType* getType() { return &m_type; };
    virtual bool isChangble() = 0;

protected:
    Type m_type;
};

template <typename Type>
class ConstVarItem : public TypedItem<Type> {
public:
    using Data = typename Type::InternalType;
    explicit ConstVarItem(const std::string& name, typename Type::InternalType constVar) :
        TypedItem<Type>(name), m_constVar(constVar){};
    virtual ~ConstVarItem() {}
    typename Type::InternalType getConstVar() const { return m_constVar; }
    virtual void dumpSymbolItem(std::ostream& os, bool hasType = true) override {
        if (hasType) {
            TypedItem<Type>::m_type.dumpType(os);
            os << " ";
        }
        os << m_constVar;
    }
    virtual bool isChangble() override { return false; }
    virtual bool hasInit() const override { return true; }

private:
    typename Type::InternalType m_constVar;
};

template <typename Type>
class VarItem : public TypedItem<Type> {
public:
    using Data = struct {
        typename Type::InternalItem items;
        typename Type::InternalType values;
        bool hasInit;
    };

    explicit VarItem(const std::string& name, Data data) :
        TypedItem<Type>(name),
        m_varItem(data.items), m_var(data.values), m_hasInit(data.hasInit){};
    virtual ~VarItem() {}
    virtual void dumpSymbolItem(std::ostream& os, bool hasType = true) override {
        if (hasType) {
            TypedItem<Type>::m_type.dumpType(os);
            os << " ";
        }
        os << m_var;
    }
    virtual bool isChangble() override { return true; }
    virtual bool hasInit() const override { return m_hasInit; }
    const typename Type::InternalItem& getVarItem() const { return m_varItem; }

private:
    typename Type::InternalItem m_varItem;
    typename Type::InternalType m_var;
    bool m_hasInit{false};
};

class IntType;
class FuncItem : public SymbolTableItem {
public:
    using Data = ValueTypeEnum;
    explicit FuncItem(const std::string& name, ValueTypeEnum retType) :
        SymbolTableItem(name), m_retType(retType){};
    std::vector<SymbolTableItem*>& getParams() { return m_params; }

    virtual ~FuncItem() {}
    virtual void dumpSymbolItem(std::ostream& os, bool hasType = true) override {
        os << (m_retType == ValueTypeEnum::INT_TYPE ? "int " : "void ");
        SymbolTableItem::dumpSymbolItem(os);
        os << "(";
        for (auto param = m_params.begin(); param != m_params.end(); param++) {
            (*param)->dumpSymbolItem(os);
            if (param != m_params.end() - 1) {
                os << ", ";
            }
        }
        os << ")";
    }
    virtual bool isChangble() override { return false; }
    virtual ValueType* getType() override { return nullptr; };
    void setParams(std::vector<SymbolTableItem*>&& params) {
        m_params.assign(params.begin(), params.end());
    }

    ValueTypeEnum getReturnValueType() const { return m_retType; }

private:
    std::vector<SymbolTableItem*> m_params;
    VarItem<IntType>* m_retValItem;
    ValueTypeEnum m_retType;
};
#endif