#include <iostream>
#include <thread>
#include <gtest/gtest.h>

#include "Lookup.hpp"

using namespace sharpsenLang;

class TokenizerTest : public ::testing::Test
{
protected:
    TokenizerTest() {}

    void SetUp() override
    {
    }

    void TearDown() override {}

    ~TokenizerTest() {}

    static void TearDownTestSuite() {}
};

TEST_F(TokenizerTest, ExampleTest)
{
   EXPECT_TRUE(true);
}
