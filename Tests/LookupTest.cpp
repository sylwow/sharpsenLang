#include <iostream>
#include <thread>
#include <gtest/gtest.h>

#include "Lookup.hpp"

using namespace sharpsenLang;

class LookupTest : public ::testing::Test
{
protected:
    LookupTest() {}

    void SetUp() override
    {
    }

    void TearDown() override {}

    ~LookupTest() {}

    static void TearDownTestSuite() {}
};

TEST_F(LookupTest, InitializeNumbersStringsSort)
{
    Lookup<int, std::string> lookup = {
        {5, "five"},
        {2, "two"},
        {2, "two (2)"},
        {6, "six"},
        {2, "two (4)"},
        {9, "nine"},
        {2, "two (3)"},
        {1, "one"},
    };

    std::vector<std::pair<int, std::string>> expected = {
        {1, "one"},
        {2, "two"},
        {2, "two (2)"},
        {2, "two (3)"},
        {2, "two (4)"},
        {5, "five"},
        {6, "six"},
        {9, "nine"},
    };

    EXPECT_EQ(expected.size(), lookup.size());

    auto lookupItem = lookup.begin();
    auto expectedItem = expected.begin();

    for (; lookupItem != lookup.end() || expectedItem != expected.end();
         lookupItem++, expectedItem++)
    {
        EXPECT_EQ(expectedItem->first, lookupItem->first);
        EXPECT_EQ(expectedItem->second, lookupItem->second);
    }
}

TEST_F(LookupTest, InitializeStringsStringsSort)
{
    Lookup<std::string, std::string> lookup = {
        {"5", "five"},
        {"2", "two"},
        {"2", "two (2)"},
        {"6", "six"},
        {"2", "two (4)"},
        {"9", "nine"},
        {"2", "two (3)"},
        {"1", "one"},
    };

    std::vector<std::pair<std::string, std::string>> expected = {
        {"1", "one"},
        {"2", "two"},
        {"2", "two (2)"},
        {"2", "two (3)"},
        {"2", "two (4)"},
        {"5", "five"},
        {"6", "six"},
        {"9", "nine"},
    };

    EXPECT_EQ(expected.size(), lookup.size());

    auto lookupItem = lookup.begin();
    auto expectedItem = expected.begin();

    for (; lookupItem != lookup.end() || expectedItem != expected.end();
         lookupItem++, expectedItem++)
    {
        EXPECT_EQ(expectedItem->first, lookupItem->first);
        EXPECT_EQ(expectedItem->second, lookupItem->second);
    }
}

TEST_F(LookupTest, FindKeySuccess)
{
    Lookup<std::string, std::string> lookup = {
        {"5", "five"},
        {"2", "two"},
        {"2", "two (2)"},
        {"6", "six"},
        {"2", "two (4)"},
        {"9", "nine"},
        {"2", "two (3)"},
        {"1", "one"},
    };

    std::pair<std::string, std::string> expected = {"5", "five"};

    auto found = lookup.find("5");

    EXPECT_EQ(expected, *found);
}

TEST_F(LookupTest, FindKeyFail)
{
    Lookup<std::string, std::string> lookup = {
        {"5", "five"},
        {"2", "two"},
        {"2", "two (2)"},
        {"6", "six"},
        {"2", "two (4)"},
        {"9", "nine"},
        {"2", "two (3)"},
        {"1", "one"},
    };

    auto expected = lookup.end();

    auto found = lookup.find("55");

    EXPECT_EQ(expected, found);
}