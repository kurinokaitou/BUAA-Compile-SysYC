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
    auto ret1 = table.insertItem<FuncItem<ArrayType<IntType>>>("hello", {.parentHandle = table.getCurrentScopeHandle(), .params = {nullptr}});
    table.pushScope(BlockScopeType::FUNC);
    auto ret2 = table.insertItem<FuncItem<ArrayType<IntType>>>("hello", {.parentHandle = table.getCurrentScopeHandle(), .params = {nullptr}});
    EXPECT_EQ(ret1.second, true);
    EXPECT_EQ(ret2.second, true);
}

TEST_F(SymbolTableTest, PushAndPopScope) {
    table.insertItem<FuncItem<ArrayType<IntType>>>("item1_in_scope1", {.parentHandle = table.getCurrentScopeHandle(), .params = {nullptr}});
    table.pushScope(BlockScopeType::FUNC);
    table.insertItem<FuncItem<ArrayType<IntType>>>("item2_in_scope2", {.parentHandle = table.getCurrentScopeHandle(), .params = {nullptr}});
    table.popScope();
    auto item1 = table.findItem("item1_in_scope1");
    auto item2 = table.findItem("item2_in_scope2");
    EXPECT_NE(item1, nullptr);
    EXPECT_EQ(item2, nullptr);
}