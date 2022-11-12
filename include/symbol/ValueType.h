#ifndef VALUE_TYPE_H
#define VALUE_TYPE_H
#include <Log.h>
#include "SymbolTableItem.h"

static const size_t INT_SIZE = 4;
static const size_t CHAR_SIZE = 1;
static const size_t VOID_SIZE = 0;
class ValueType {
public:
    using InteralType = int;
    virtual size_t valueSize() const { return 0; };
    virtual ValueTypeEnum getValueTypeEnum() = 0;
    virtual bool isArray() = 0;
    virtual void dumpType(std::ostream& os) const = 0;
};

class IntType : public ValueType {
public:
    using InternalType = int;
    using InternalItem = SymbolTableItem*;
    virtual size_t valueSize() const override { return INT_SIZE; }
    virtual ValueTypeEnum getValueTypeEnum() override {
        return ValueTypeEnum::INT_TYPE;
    }
    virtual bool isArray() override { return false; }
    virtual void dumpType(std::ostream& os) const override {
        os << "i32";
    }
};

class CharType : public ValueType {
public:
    using InternalType = char;
    using InternalItem = SymbolTableItem*;
    virtual size_t valueSize() const override { return CHAR_SIZE; }
    virtual ValueTypeEnum getValueTypeEnum() override {
        return ValueTypeEnum::CHAR_TYPE;
    }
    virtual bool isArray() override { return false; }
    virtual void dumpType(std::ostream& os) const override {
        os << "i8";
    }
};

class VoidType : public ValueType {
public:
    using InternalType = void*;
    using InternalItem = SymbolTableItem*;
    virtual size_t valueSize() const override { return VOID_SIZE; }
    virtual ValueTypeEnum getValueTypeEnum() override {
        return ValueTypeEnum::VOID_TYPE;
    }
    virtual bool isArray() override { return false; }
    virtual void dumpType(std::ostream& os) const override {
        os << "void";
    }
};

template <typename T>
class MultiFlatArray {
public:
    using Data = struct MultiFlatArrayData {
        std::vector<T> values;
        std::vector<size_t> dimensions;
    };
    MultiFlatArray() = default;
    MultiFlatArray(Data data) :
        m_data(data) {
    }
    MultiFlatArray(T val, size_t n) {
        m_data.values.assign(val, n);
    }

    std::vector<T>& getValues() { return m_data.values; }

    T& operator[](std::vector<size_t>&& pos) {
        size_t index = 0;
        int diff = m_data.dimensions.size() - pos.size();
        for (int i = 0; i < diff; i++) {
            pos.push_back(0);
        }
        for (int i = 0; i < m_data.dimensions.size(); i++) {
            size_t term = 1;
            for (int j = i + 1; j < m_data.dimensions.size(); j++) {
                term *= m_data.dimensions[j];
            }
            index += pos[i] * term;
        }
        return m_data.values[index];
    }

    friend std::ostream& operator<<(std::ostream& os, const MultiFlatArray<T>& array) {
        if (!array.m_data.values.empty()) {
            os << "[";
            for (auto val = array.m_data.values.begin(); val != array.m_data.values.end(); val++) {
                os << "i" << sizeof(T) * 8 << " " << *val;
                if (val != array.m_data.values.end() - 1) {
                    os << ", ";
                }
            }
            os << "]";
        }
        return os;
    }

    void append(MultiFlatArray<T>&& appendance) {
        m_data.values.insert(m_data.values.end(), appendance.m_data.values.begin(), appendance.m_data.values.end());
    }

    void insert(T val) {
        m_data.values.push_back(val);
    }

    void setDimensions(std::vector<size_t>& dims) {
        m_data.dimensions.assign(dims.begin(), dims.end());
    }

    const std::vector<size_t>& getDimensions() const {
        return m_data.dimensions;
    }

    size_t spaceSize(size_t unitSize) {
        size_t count = unitSize;
        for (size_t dimension : m_data.dimensions) {
            count *= dimension;
        }
        return count;
    }

private:
    Data m_data;
};

template <typename Type>
class ArrayType : public ValueType {
public:
    using InternalType = MultiFlatArray<typename Type::InternalType>;
    using InternalItem = MultiFlatArray<SymbolTableItem*>;
    virtual size_t valueSize() const override {
        return m_basicType.valueSize();
    }
    virtual ValueTypeEnum getValueTypeEnum() override {
        return m_basicType.getValueTypeEnum();
    }
    virtual bool isArray() override { return true; }
    virtual void dumpType(std::ostream& os) const override {
        m_basicType.dumpType(os);
    }

private:
    Type m_basicType;
};

template <typename Type>
static std::vector<size_t> getArrayItemDimensions(SymbolTableItem* item) {
    if (item->isChangble()) {
        auto lValArray = dynamic_cast<VarItem<ArrayType<Type>>*>(item);
        if (lValArray) {
            return lValArray->getVarItem().getDimensions();
        } else {
            return {};
        }

    } else {
        auto lValArray = dynamic_cast<ConstVarItem<ArrayType<Type>>*>(item);
        if (lValArray) {
            return lValArray->getConstVar().getDimensions();
        } else {
            return {};
        }
    }
}

static std::vector<size_t> getArrayItemDimensions(SymbolTableItem* item) {
    if (item->getType()->getValueTypeEnum() == ValueTypeEnum::INT_TYPE) {
        return getArrayItemDimensions<IntType>(item);
    } else {
        return getArrayItemDimensions<CharType>(item);
    }
}

static size_t calArrayDimsSize(std::vector<size_t>& dims) {
    size_t size = 1;
    if (dims.empty()) {
        return 0;
    } else {
        for (auto dim : dims) {
            size *= dim;
        }
        return size;
    }
}

static std::vector<size_t> calAccDimensions(std::vector<size_t>& dims) {
    std::vector<size_t> accDims;
    accDims.reserve(dims.size());
    for (int i = 0; i < dims.size(); i++) {
        int acc = 1;
        for (int j = i + 1; j < dims.size(); j++) {
            acc *= dims[j];
        }
        accDims.push_back(acc);
    }
    return accDims;
}

template <typename T, size_t N>
class MultiArray {
public:
    using InteralType = MultiArray<T, N - 1>;
    MultiArray() = default;
    MultiArray(std::initializer_list<InteralType> list) {
        m_values.assign(list);
    }
    void push_back(InteralType&& value) {
        m_values.push_back(std::forward<InteralType>(value));
    }
    friend std::ostream& operator<<(std::ostream& os, MultiArray<T, N>& arr) {
        os << "{";
        for (auto it = arr.m_values.begin(); it != arr.m_values.end(); it++) {
            os << *it;
            if (it != arr.m_values.end() - 1) {
                os << ", ";
            }
        }
        os << "}";
        return os;
    }
    MultiArray<T, N - 1>& operator[](size_t index) {
        return m_values[index];
    }

private:
    std::vector<InteralType> m_values;
};

template <typename T>
class MultiArray<T, 1> {
public:
    using InteralType = typename T::InternalType;
    MultiArray() = default;
    MultiArray(std::initializer_list<InteralType> list) {
        m_values.assign(list);
    }
    void push_back(InteralType value) {
        m_values.push_back(value);
    }
    friend std::ostream& operator<<(std::ostream& os, MultiArray<T, 1>& arr) {
        os << "{";
        for (auto it = arr.m_values.begin(); it != arr.m_values.end(); it++) {
            os << *it;
            if (it != arr.m_values.end() - 1) {
                os << ", ";
            }
        }
        os << "}";
        return os;
    }
    InteralType operator[](size_t index) {
        return m_values[index];
    }

private:
    std::vector<InteralType> m_values;
};

template <typename T>
class MultiArray<T, 0> {
public:
    using InteralType = typename T::InternalType;
    MultiArray() = default;
    void push_back(InteralType&& value) {}
    friend std::ostream& operator<<(std::ostream& os, MultiArray<T, 0>& arr) { return os; }
    int operator[](size_t index) { return 0; }
};

#endif