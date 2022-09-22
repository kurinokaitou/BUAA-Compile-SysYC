#ifndef SYMBOL_TABLE_TEST_H
#define SYMBOL_TABLE_TEST_H
#include <gtest/gtest.h>
#include <symbol/SymbolTable.h>

class SymbolTableTest : public ::testing::Test {
protected:
    SymbolTableTest() = default;
    virtual ~SymbolTableTest() = default;
    virtual void SetUp(){};
    virtual void TearDown() {
        table.clearSymbolTable();
    };
    static void SetUpTestSuite() {
        std::cout << "Start testing symbol table" << std::endl;
    }

    static void TearDownTestSuite() {
        std::cout << "End testing symbol table" << std::endl;
    }
    SymbolTable table;
};
#endif