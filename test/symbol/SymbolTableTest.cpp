#include "SymbolTableTest.h"
#include <iostream>

TEST_F(SymbolTableTest, SimpleInsert) {
    table.insertItem<FuncItem<ArrayType<IntType>>, FuncItem<ArrayType<IntType>>::Data>("hello", {.parentHandle = BlockScopeHandle(0), .params = {}});
    table.pushScope(BlockScopeType::FUNC);
    table.insertItem<VarItem<IntType>, VarItem<IntType>::Data>("a", {.parentHandle = BlockScopeHandle(1), .var = 1});
    EXPECT_EQ(table.getCurrentScope().countSymbol(), 1);
    EXPECT_EQ(table.findItem("hello")->getLevel(), 0);
    EXPECT_EQ(table.findItem("a")->getLevel(), 1);
}

// TEST_F(SymbolTableTest, InsertConfirm) {
//     EXPECT_NE(table.findItem("hello"), nullptr);
// }