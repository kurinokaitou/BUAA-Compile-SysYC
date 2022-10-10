#ifndef ERROR_TEST_H
#define ERROR_TEST_H
#include <gtest/gtest.h>
#include <Compiler.h>

class ErrorTest : public ::testing::Test {
protected:
    ErrorTest() = default;
    virtual ~ErrorTest() = default;
    virtual void SetUp(){

    };
    virtual void TearDown(){

    };
    Compiler m_compiler;
    std::filebuf in;
};
#endif