#include "SymbolTableTest.h"
#include <iostream>

TEST_F(SymbolTableTest, SimpleInsert) {
    auto ret = table.insertItem<FuncItem<ArrayType<IntType>>>("hello", {.parentHandle = BlockScopeHandle(0), .params = {nullptr}});
    EXPECT_EQ(dynamic_cast<FuncItem<ArrayType<IntType>>*>(ret.first)->getParams()[0], nullptr);
    table.pushScope(BlockScopeType::FUNC);
    table.insertItem<VarItem<IntType>>("a", {.parentHandle = BlockScopeHandle(1), .var = 1});
    EXPECT_EQ(table.getCurrentScope().countSymbol(), 1);
    EXPECT_EQ(table.findItem("hello")->getLevel(), 0);
    EXPECT_EQ(table.findItem("a")->getLevel(), 1);
}

TEST_F(SymbolTableTest, DoubleInsertInSameScope) {
    auto ret1 = table.insertItem<FuncItem<ArrayType<IntType>>>("hello", {.parentHandle = BlockScopeHandle(0), .params = {nullptr}});
    auto ret2 = table.insertItem<FuncItem<ArrayType<IntType>>>("hello", {.parentHandle = BlockScopeHandle(0), .params = {nullptr}});
    EXPECT_EQ(ret1.second, true);
    EXPECT_EQ(ret2.second, false);
}

TEST_F(SymbolTableTest, DoubleInsertInDiffScope) {
    auto ret1 = table.insertItem<FuncItem<ArrayType<IntType>>>("hello", {.parentHandle = BlockScopeHandle(0), .params = {nullptr}});
    table.pushScope(BlockScopeType::FUNC);
    auto ret2 = table.insertItem<FuncItem<ArrayType<IntType>>>("hello", {.parentHandle = BlockScopeHandle(0), .params = {nullptr}});
    EXPECT_EQ(ret1.second, true);
    EXPECT_EQ(ret2.second, true);
}