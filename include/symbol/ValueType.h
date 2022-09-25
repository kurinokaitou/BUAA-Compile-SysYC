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

static const size_t INT_SIZE = 4;
static const size_t VOID_SIZE = 0;
class ValueType {
public:
    virtual size_t countSize() const = 0;
    virtual ValueTypeEnum getValueTypeEnum() = 0;
};

class IntType : public ValueType {
public:
    using InternalType = int;
    virtual size_t countSize() const override { return INT_SIZE; }
    virtual ValueTypeEnum getValueTypeEnum() override {
        return ValueTypeEnum::INT_TYPE;
    }
};

class VoidType : public ValueType {
public:
    using InternalType = void;
    virtual size_t countSize() const override { return VOID_SIZE; }
    virtual ValueTypeEnum getValueTypeEnum() override {
        return ValueTypeEnum::VOID_TYPE;
    }
};

template <typename Type>
class ArrayType : public ValueType {
public:
    using InternalType = std::vector<typename Type::InternalType>;
    virtual size_t countSize() const override {
        size_t count = m_basicType.countSize();
        for (size_t dimension : m_dimensions) {
            count *= dimension;
        }
        return count;
    }
    virtual ValueTypeEnum getValueTypeEnum() override {
        return ValueTypeEnum::ARRAY_TYPE;
    }
    size_t getValueIndex(std::vector<size_t>&& pos) {
        size_t index = 0;
        int diff = m_dimensions.size() - pos.size();
        for (int i = 0; i < diff; i++) {
            pos.push_back(0);
        }
        for (int i = 0; i < m_dimensions.size(); i++) {
            size_t term = 1;
            for (int j = i + 1; j < m_dimensions.size(); j++) {
                term *= m_dimensions[j];
            }
            index += pos[i] * term;
        }
        return index;
    }

    void setDimension(std::initializer_list<size_t> dimensions) {
        m_dimensions.assign(dimensions);
    }

private:
    std::vector<size_t> m_dimensions;
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