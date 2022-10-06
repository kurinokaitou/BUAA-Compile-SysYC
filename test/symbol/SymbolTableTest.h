#ifndef SYMBOL_TABLE_TEST_H
#define SYMBOL_TABLE_TEST_H
#include <gtest/gtest.h>
#include <symbol/SymbolTable.h>
#include <symbol/ValueType.h>

class SymbolTableTest : public ::testing::Test {
protected:
    SymbolTableTest() = default;
    virtual ~SymbolTableTest() = default;
    virtual void SetUp() {
        table.initSymbolTable();
    };
    virtual void TearDown() {
        table.clearSymbolTable();
    };
    SymbolTable table;
};
#endif