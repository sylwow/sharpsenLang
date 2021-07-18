#include <iostream>
#include <thread>
#include <gtest/gtest.h>

#include "Types.hpp"

using namespace sharpsenLang;
class TypeRegistryTest : public ::testing::Test
{
protected:
    TypeRegistryTest() {}

    void SetUp() override {}

    void TearDown() override {}

    ~TypeRegistryTest() {}

    static void TearDownTestSuite() {}

    TypeRegistry tr;
};

TEST_F(TypeRegistryTest, CheckVoid)
{
    auto nothing = tr.getHandle(SimpleType::Void);

    EXPECT_EQ(tr.getVoidHandle(), nothing);
}

TEST_F(TypeRegistryTest, CheckString)
{
    auto string = tr.getHandle(SimpleType::String);

    EXPECT_EQ(tr.getStringHandle(), string);
}

TEST_F(TypeRegistryTest, CheckNumber)
{
    auto number = tr.getHandle(SimpleType::Number);

    EXPECT_EQ(tr.getNumberHandle(), number);
}

TEST_F(TypeRegistryTest, CheckNumberArray)
{
    ArrayType numberArray{ tr.getNumberHandle() };

    auto array = tr.getHandle(numberArray);

    EXPECT_EQ(tr.getHandle(numberArray), array);
}

TEST_F(TypeRegistryTest, CheckFailArray)
{
    ArrayType numberArray{ tr.getNumberHandle() };

    auto array = tr.getHandle(numberArray);

    EXPECT_NE(tr.getHandle(ArrayType{tr.getStringHandle()}), array);
}


TEST_F(TypeRegistryTest, CheckFunction)
{
    FunctionType funct{ tr.getNumberHandle(), {{tr.getNumberHandle(), false}, {tr.getStringHandle(), true}} };

    auto funcType = tr.getHandle(funct);

    EXPECT_EQ(tr.getHandle(funct), funcType);
}

TEST_F(TypeRegistryTest, CheckFailFunction)
{
    FunctionType funct{ tr.getNumberHandle(), {{tr.getNumberHandle(), false}, {tr.getStringHandle(), true}} };

    auto funcType = tr.getHandle(funct);

    FunctionType funct2{ tr.getNumberHandle(), {{tr.getNumberHandle(), false}, {tr.getNumberHandle(), true}} };
    EXPECT_NE(tr.getHandle(funct2), funcType);
}
