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
    ArrayType numberArray{tr.getNumberHandle()};

    auto array = tr.getHandle(numberArray);

    EXPECT_EQ(tr.getHandle(numberArray), array);
}

TEST_F(TypeRegistryTest, CheckFailArray)
{
    ArrayType numberArray{tr.getNumberHandle()};

    auto array = tr.getHandle(numberArray);

    EXPECT_NE(tr.getHandle(ArrayType{tr.getStringHandle()}), array);
}

TEST_F(TypeRegistryTest, CheckFunction)
{
    FunctionType funct{tr.getNumberHandle(), {{tr.getNumberHandle(), false}, {tr.getStringHandle(), true}}};

    auto funcType = tr.getHandle(funct);

    EXPECT_EQ(tr.getHandle(funct), funcType);
}

TEST_F(TypeRegistryTest, CheckFailFunction)
{
    FunctionType funct{tr.getNumberHandle(), {{tr.getNumberHandle(), false}, {tr.getStringHandle(), true}}};

    auto funcType = tr.getHandle(funct);

    FunctionType funct2{tr.getNumberHandle(), {{tr.getNumberHandle(), false}, {tr.getNumberHandle(), true}}};
    EXPECT_NE(tr.getHandle(funct2), funcType);
}
/*
TEST_F(TypeRegistryTest, CheckClass)
{
    ClassType myClass{"myClass", "namespace.myClass", {tr.getNumberHandle()}};

    auto myClassType = tr.getHandle(myClass);

    EXPECT_EQ(tr.getHandle(myClass), myClassType);
}

TEST_F(TypeRegistryTest, CheckFailClass)
{
    ClassType myClass{"myClass", "namespace.myClass", {tr.getNumberHandle()}};

    auto myClassType = tr.getHandle(myClass);

    ClassType myClass2{"myClass2", "namespace.myClass2", {tr.getNumberHandle()}};

    EXPECT_NE(tr.getHandle(myClass2), myClassType);
}

TEST_F(TypeRegistryTest, CheckNonRegisteredClass)
{

    EXPECT_FALSE(tr.getRegisteredClassHandle("myClass"));
}

TEST_F(TypeRegistryTest, CheckClassRegistration)
{
    ClassType myClass{"myClass", "namespace.myClass", {tr.getNumberHandle()}};

    auto myClassType = tr.getHandle(myClass);

    EXPECT_EQ(tr.checkClassRegistration("namespace.myClass", 1, 2), myClassType);
}

TEST_F(TypeRegistryTest, CheckClassNonRegistration)
{
    EXPECT_FALSE(tr.checkClassRegistration("namespace.myClass", 1, 2));

    auto unregistered = tr.getUndefinedTypes();

    EXPECT_EQ(unregistered.size(), 1);

    UndefinedInfo &ui = unregistered.at(0);
    EXPECT_EQ(ui.type, "namespace.myClass");
    EXPECT_EQ(ui.lineNumber, 1);
    EXPECT_EQ(ui.charIndex, 2);
}

TEST_F(TypeRegistryTest, CheckClassNonRegistration2)
{
    ClassType myClass{"myClass", "namespace.myClass", {tr.getNumberHandle()}};

    EXPECT_FALSE(tr.checkClassRegistration("namespace.myClass", 1, 2));

    auto myClassType = tr.getHandle(myClass);

    auto unregistered = tr.getUndefinedTypes();

    EXPECT_EQ(unregistered.size(), 0);
}

TEST_F(TypeRegistryTest, CheckClassNonRegistrationMultiple)
{
    ClassType myClass{"myClass", "namespace.myClass", {tr.getNumberHandle()}};

    EXPECT_FALSE(tr.checkClassRegistration("namespace.myClass", 1, 2));
    EXPECT_FALSE(tr.checkClassRegistration("namespace.myClass", 12, 22));
    EXPECT_FALSE(tr.checkClassRegistration("namespace.myClass", 111, 244));

    auto unregistered = tr.getUndefinedTypes();

    EXPECT_EQ(unregistered.size(), 1);

    UndefinedInfo &ui = unregistered.at(0);
    EXPECT_EQ(ui.type, "namespace.myClass");
    EXPECT_EQ(ui.lineNumber, 1);
    EXPECT_EQ(ui.charIndex, 2);
}
*/