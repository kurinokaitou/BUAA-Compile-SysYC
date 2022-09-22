#ifndef VALUE_TYPE_H
#define VALUE_TYPE_H
#include <vector>
#include <memory>
class ValueType {
public:
    virtual int countSize() const = 0;
};

template <typename Type>
class ArrayType : public ValueType {
public:
    virtual int countSize() const override {
        int count = 1;
        for (int dimension : m_dimensions) {
            count *= dimension;
        }
        return count;
    }

    void setDimension(std::initializer_list<int> dimensions) {
        m_dimensions.assign(dimensions.begin(), dimensions.end());
    }

private:
    std::vector<int> m_dimensions;
    Type m_basicType;
};

class IntType : public ValueType {
public:
    using InternalType = int;
    virtual int countSize() const override { return 4; }
};

class VoidType : public ValueType {
public:
    using InternalType = void;
    virtual int countSize() const override { return 0; }
};
#endif