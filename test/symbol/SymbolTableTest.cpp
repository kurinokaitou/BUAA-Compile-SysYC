#include "SymbolTableTest.h"
#include <iostream>

TEST_F(SymbolTableTest, SimpleInsert) {
    auto ret = table.insertFunc("func1", ValueTypeEnum::INT_TYPE);
    table.pushScope(BlockScopeType::FUNC);
    table.insertItem<ConstVarItem<IntType>>("a", 1);
    EXPECT_EQ(table.getCurrentScope().countSymbol(), 1);
    EXPECT_EQ(table.findItem("a")->getLevel(), 1);
    EXPECT_EQ(table.findFunc("func1")->getLevel(), 0);
}

TEST_F(SymbolTableTest, DoubleInsertInSameScope) {
    auto ret1 = table.insertFunc("hello", ValueTypeEnum::INT_TYPE);
    auto ret2 = table.insertFunc("hello", ValueTypeEnum::VOID_TYPE);
    EXPECT_EQ(ret1.second, true);
    EXPECT_EQ(ret2.second, false);
}

TEST_F(SymbolTableTest, DoubleInsertInDiffScope) {
    auto ret2 = table.insertItem<ConstVarItem<IntType>>("hello", 2);
    auto ret1 = table.insertFunc("hello", ValueTypeEnum::INT_TYPE);
    table.pushScope(BlockScopeType::FUNC);
    EXPECT_EQ(ret1.second, true);
    EXPECT_EQ(ret2.second, true);
}

TEST_F(SymbolTableTest, PushAndPopScope) {
    table.insertFunc("item1_in_scope1", ValueTypeEnum::INT_TYPE);
    table.insertFunc("item2_in_scope2", ValueTypeEnum::VOID_TYPE);
    auto item1 = table.findFunc("item1_in_scope1");
    auto item2 = table.findFunc("item2_in_scope2");
    EXPECT_NE(item1, nullptr);
    EXPECT_NE(item2, nullptr);
}

TEST_F(SymbolTableTest, ArrayTypeInsert) {
    table.insertItem<ConstVarItem<ArrayType<IntType>>>("const int arr", {{.values = {1, 3, 0, 4, 0, 0, 1, 0, 0}, .dimensions = {3, 3}}});
    SymbolTableItem* item = table.findItem("const int arr");
    auto arri2 = dynamic_cast<ConstVarItem<ArrayType<IntType>>*>(item);

    EXPECT_EQ((arri2->getConstVar()[{0, 0}]), 1);
    EXPECT_EQ((arri2->getConstVar()[{0, 1}]), 3);
    EXPECT_EQ((arri2->getConstVar()[{1, 0}]), 4);
    EXPECT_EQ((arri2->getConstVar()[{1, 1}]), 0);
}