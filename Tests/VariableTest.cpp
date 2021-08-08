#include <iostream>
#include <thread>
#include <gtest/gtest.h>

#include "Variable.hpp"

using namespace sharpsenLang;

class VariableTest : public ::testing::Test
{
protected:
    VariableTest() {}

    void SetUp() override
    {
    }

    void TearDown() override {}

    ~VariableTest() {}

    template <class T, class... Args>
    T makeVariable(Args... args)
    {
        return std::make_shared<VariableImpl<T::element_type::ValueType>>(args...);
    }

    template <class T, class... Args>
    T makeVariable(std::initializer_list<Args...> args)
    {
        return std::make_shared<VariableImpl<T::element_type::ValueType>>(args);
    }

    static void TearDownTestSuite() {}
};

TEST_F(VariableTest, Number)
{
    Lnumber number = makeVariable<Lnumber>(1);

    double expected = 1;
    EXPECT_EQ(expected, number->value);
}

TEST_F(VariableTest, String)
{
    Lstring string = makeVariable<Lstring>(std::make_shared<std::string>("abcd"));

    std::string expected = "abcd";
    EXPECT_EQ(expected, *(string->value));
}

TEST_F(VariableTest, Array)
{
    std::initializer_list<VariablePtr> initializerList = {makeVariable<Lnumber>(1), makeVariable<Lnumber>(2), makeVariable<Lnumber>(3)};
    Larray array = makeVariable<Larray>(initializerList);

    std::deque<VariablePtr> expected = {makeVariable<Lnumber>(1), makeVariable<Lnumber>(2), makeVariable<Lnumber>(3)};
    for (size_t i = 0; i < expected.size(); i++)
    {
        auto expectedValue = expected.at(i)->staticPointerDowncast<Lnumber>()->value;
        auto value = array->value.at(i)->staticPointerDowncast<Lnumber>()->value;
        EXPECT_EQ(expectedValue, value);
    }
}

TEST_F(VariableTest, Touple)
{
    std::initializer_list<VariablePtr> initializerList = {makeVariable<Lnumber>(1), makeVariable<Lnumber>(2), makeVariable<Lnumber>(3)};
    Ltuple array = makeVariable<Ltuple>(initializerList);

    std::deque<VariablePtr> expected = {makeVariable<Lnumber>(1), makeVariable<Lnumber>(2), makeVariable<Lnumber>(3)};
    for (size_t i = 0; i < expected.size(); i++)
    {
        auto expectedValue = expected.at(i)->staticPointerDowncast<Lnumber>()->value;
        auto value = array->value.at(i)->staticPointerDowncast<Lnumber>()->value;
        EXPECT_EQ(expectedValue, value);
    }
}

TEST_F(VariableTest, NumberCopy)
{
    Lnumber number = makeVariable<Lnumber>(1);

    auto cloned = number->clone();

    EXPECT_NE(number, cloned);

    double expected = 1;
    EXPECT_EQ(expected, cloned->staticPointerDowncast<Lnumber>()->value);
}

TEST_F(VariableTest, StringCopy)
{
    Lstring string = makeVariable<Lstring>(std::make_shared<std::string>("abcd"));


    auto cloned = string->clone();

    EXPECT_NE(string, cloned);

    std::string expected = "abcd";
    EXPECT_EQ(expected, *(cloned->staticPointerDowncast<Lstring>()->value));
}

TEST_F(VariableTest, ArrayCopy)
{
    std::initializer_list<VariablePtr> initializerList = {makeVariable<Lnumber>(1), makeVariable<Lnumber>(2), makeVariable<Lnumber>(3)};
    Larray array = makeVariable<Larray>(initializerList);

    auto cloned = array->clone();

    EXPECT_NE(array, cloned);

    std::deque<VariablePtr> expected = {makeVariable<Lnumber>(1), makeVariable<Lnumber>(2), makeVariable<Lnumber>(3)};
    for (size_t i = 0; i < expected.size(); i++)
    {
        auto expectedValue = expected.at(i)->staticPointerDowncast<Lnumber>()->value;
        auto value = array->value.at(i)->staticPointerDowncast<Lnumber>()->value;
        EXPECT_EQ(expectedValue, value);
    }
}

TEST_F(VariableTest, ToupleCopy)
{
    std::initializer_list<VariablePtr> initializerList = {makeVariable<Lnumber>(1), makeVariable<Lnumber>(2), makeVariable<Lnumber>(3)};
    Ltuple touple = makeVariable<Ltuple>(initializerList);

    auto cloned = touple->clone();

    EXPECT_NE(touple, cloned);

    std::deque<VariablePtr> expected = {makeVariable<Lnumber>(1), makeVariable<Lnumber>(2), makeVariable<Lnumber>(3)};
    for (size_t i = 0; i < expected.size(); i++)
    {
        auto expectedValue = expected.at(i)->staticPointerDowncast<Lnumber>()->value;
        auto value = touple->value.at(i)->staticPointerDowncast<Lnumber>()->value;
        EXPECT_EQ(expectedValue, value);
    }
}

TEST_F(VariableTest, FunctionCopy)
{
    Lfunction function = makeVariable<Lfunction>([](RuntimeContext&){});

    auto cloned = function->clone();

    EXPECT_NE(function, cloned);
}

TEST_F(VariableTest, NumberToString)
{
    Lnumber number = makeVariable<Lnumber>(1);

    String stringValue = number->toString();

    std::string expected = "1";
    EXPECT_EQ(expected, *stringValue);
}

TEST_F(VariableTest, StringToString)
{
    Lstring string = makeVariable<Lstring>(std::make_shared<std::string>("abcd"));

    String stringValue = string->toString();

    std::string expected = "abcd";
    EXPECT_EQ(expected, *stringValue);
}

TEST_F(VariableTest, ArrayToString)
{
    std::initializer_list<VariablePtr> initializerList = {makeVariable<Lnumber>(1), makeVariable<Lnumber>(2), makeVariable<Lnumber>(3)};
    Larray array = makeVariable<Larray>(initializerList);

    String stringValue = array->toString();

    std::string expected = "[1, 2, 3]";
    EXPECT_EQ(expected, *stringValue);
}

TEST_F(VariableTest, ToupleToString)
{
    std::initializer_list<VariablePtr> initializerList = {makeVariable<Lnumber>(1), makeVariable<Lnumber>(2), makeVariable<Lnumber>(3)};
    Ltuple touple = makeVariable<Ltuple>(initializerList);

    String stringValue = touple->toString();

    std::string expected = "[1, 2, 3]";
    EXPECT_EQ(expected, *stringValue);
}

TEST_F(VariableTest, FunctionToString)
{
    Lfunction function = makeVariable<Lfunction>([](RuntimeContext&){});

    String stringValue = function->toString();

    std::string expected = "FUNCTION";
    EXPECT_EQ(expected, *stringValue);
}