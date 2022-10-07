#ifndef VALUE_TYPE_H
#define VALUE_TYPE_H
#include <Log.h>
#include "SymbolTableItem.h"

static const size_t INT_SIZE = 4;
static const size_t VOID_SIZE = 0;
class ValueType {
public:
    using InteralType = int;
    size_t valueSize(InteralType value) const { return 0; };
    virtual ValueTypeEnum getValueTypeEnum() = 0;
};

class IntType : public ValueType {
public:
    using InternalType = int;
    using InternalItem = VarItem<IntType>*;
    size_t valueSize(InternalType value) const { return INT_SIZE; }
    virtual ValueTypeEnum getValueTypeEnum() override {
        return ValueTypeEnum::INT_TYPE;
    }
    friend std::ostream& operator<<(std::ostream& os, const IntType& type) {
        os << "int";
        return os;
    }
};

class VoidType : public ValueType {
public:
    using InternalType = unsigned int; // 无法存储一个void类型的成员变量，因此用uint区分
    size_t valueSize(InternalType) const { return VOID_SIZE; }
    virtual ValueTypeEnum getValueTypeEnum() override {
        return ValueTypeEnum::VOID_TYPE;
    }
    friend std::ostream& operator<<(std::ostream& os, const VoidType& type) {
        os << "void";
        return os;
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
        for (auto dim : array.m_data.dimensions) {
            os << "[" << dim << "]";
        }
        os << " ";
        for (auto val : array.m_data.values) {
            os << val << " ";
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
    using InternalItem = MultiFlatArray<VarItem<Type>*>;
    size_t valueSize(InternalType value) const {
        typename Type::InteralType dummy;
        size_t count = m_basicType.valueSize(dummy);
        return value.spaceSize(count);
    }
    virtual ValueTypeEnum getValueTypeEnum() override {
        return ValueTypeEnum::ARRAY_TYPE;
    }

    friend std::ostream& operator<<(std::ostream& os, const ArrayType& type) {
        os << "array<" << type.m_basicType << ">";
        return os;
    }

private:
    Type m_basicType;
};

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