#ifndef VALUE_TYPE_H
#define VALUE_TYPE_H
#include <vector>
#include <memory>

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

template <typename Type, size_t N>
class ArrayType : public ValueType {
public:
    using InternalType = std::vector<typename Type::InternalType>;
    virtual int countSize() const override {
        int count = 1;
        for (int dimension : m_dimensions) {
            count *= dimension;
        }
        return count;
    }
    virtual ValueTypeEnum getValueTypeEnum() override {
        return ValueTypeEnum::ARRAY_TYPE;
    }

    void setDimension(std::initializer_list<int> dimensions) {
        m_dimensions.assign(dimensions.begin(), dimensions.end());
    }

private:
    std::vector<int> m_dimensions;
    Type m_basicType;
};

template <typename Type>
class ArrayType<Type, 1> : public ValueType {
public:
    using InternalType = std::vector<typename Type::InternalType>;
    virtual int countSize() const override {
        return m_dx * m_basicType.countSize();
    }
    virtual ValueTypeEnum getValueTypeEnum() override {
        return ValueTypeEnum::ARRAY_TYPE;
    }

    void setDimension(int x) {
        m_dx = x;
    }

private:
    size_t m_dx{0};
    Type m_basicType;
};

template <typename Type>
class ArrayType<Type, 2> : public ValueType {
public:
    using InternalType = std::vector<std::vector<typename Type::InternalType>>;
    virtual int countSize() const override {
        return m_dx * m_dy * m_basicType.countSize();
    }
    virtual ValueTypeEnum getValueTypeEnum() override {
        return ValueTypeEnum::ARRAY_TYPE;
    }

    void setDimension(int x, int y) {
        m_dx = x;
        m_dy = y;
    }

private:
    size_t m_dx{0};
    size_t m_dy{0};
    Type m_basicType;
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

using ArrayI2 = ArrayType<IntType, 2>;
using ArrayI1 = ArrayType<IntType, 1>;
#endif