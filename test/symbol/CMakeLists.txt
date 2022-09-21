find_package(GTest CONFIG REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})
set(TEST_NAME "symbol_test")

add_executable(${TEST_NAME} SymbolTableTest.h main.cpp SymbolTableTest.cpp 
${CMAKE_SOURCE_DIR}/src/symbol/BlockScope.cpp
${CMAKE_SOURCE_DIR}/src/symbol/SymbolTable.cpp)
target_link_libraries(${TEST_NAME} GTest::gtest GTest::gmock pthread)
target_include_directories(${TEST_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/include)
add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})