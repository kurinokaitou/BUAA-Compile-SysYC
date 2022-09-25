#ifndef VALUE_TYPE_H
#define VALUE_TYPE_H
#include <vector>
#include <memory>
#include <iostream>
#include <algorithm>

enum class ValueTypeEnum : unsigned int {
    INT_TYPE = 0,
    ARRAY_TYPE,
    VOID_TYPE,
};

static const int INT_SIZE = 4;
static const int VOID_SIZE = 0;
class ValueType {
public:
    virtual int countSize() const = 0;
    virtual ValueTypeEnum getValueTypeEnum() = 0;
};

class IntType : public ValueType {
public:
    using InternalType = int;
    virtual int countSize() const override { return INT_SIZE; }
    virtual ValueTypeEnum getValueTypeEnum() override {
        return ValueTypeEnum::INT_TYPE;
    }
};

class VoidType : public ValueType {
public:
    using InternalType = void;
    virtual int countSize() const override { return VOID_SIZE; }
    virtual ValueTypeEnum getValueTypeEnum() override {
        return ValueTypeEnum::VOID_TYPE;
    }
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
    int operator[](size_t index) {
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

template <typename Type, size_t N>
class ArrayType : public ValueType {
public:
    using InternalType = MultiArray<Type, N>;
    virtual int countSize() const override {
        int count = m_basicType.countSize();
        for (int dimension : m_dimensions) {
            count *= dimension;
        }
        return count;
    }
    virtual ValueTypeEnum getValueTypeEnum() override {
        return ValueTypeEnum::ARRAY_TYPE;
    }

    void setDimension(const std::array<int, N>&& dimensions) {
        std::copy(dimensions.begin(), dimensions.end(), m_dimensions.begin());
    }

private:
    std::array<int, N> m_dimensions;
    Type m_basicType;
};

using ArrayI2 = ArrayType<IntType, 2>;
using ArrayI1 = ArrayType<IntType, 1>;
#endif