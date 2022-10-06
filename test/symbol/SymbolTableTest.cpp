#include "SymbolTableTest.h"
#include <iostream>

TEST_F(SymbolTableTest, SimpleInsert) {
    auto ret = table.insertItem<FuncItem<ArrayType<IntType>>>("hello", {.parentHandle = BlockScopeHandle(0), .params = {nullptr}});

    table.pushScope(BlockScopeType::FUNC);
    table.insertItem<ConstVarItem<IntType>>("a", {.parentHandle = BlockScopeHandle(1), .constVar = 1});
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

TEST_F(SymbolTableTest, ArrayTypeInsert) {
    table.insertItem<ConstVarItem<ArrayType<IntType>>>("const int arr", {.parentHandle = table.getCurrentScopeHandle(),
                                                                         .constVar = {{.values = {1, 3, 0, 4, 0, 0, 1, 0, 0}, .dimensions = {3, 3}}}});
    SymbolTableItem* item = table.findItem("const int arr");
    auto arri2 = dynamic_cast<ConstVarItem<ArrayType<IntType>>*>(item);

    EXPECT_EQ(arri2->getSize(), 36);
    EXPECT_EQ((arri2->getConstVar()[{0, 0}]), 1);
    EXPECT_EQ((arri2->getConstVar()[{0, 1}]), 3);
    EXPECT_EQ((arri2->getConstVar()[{1, 0}]), 4);
    EXPECT_EQ((arri2->getConstVar()[{1, 1}]), 0);
}