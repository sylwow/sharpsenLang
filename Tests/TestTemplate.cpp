#include <iostream>
#include <thread>
#include <gtest/gtest.h>

#include "Lookup.hpp"

using namespace sharpsenLang;
class Template : public ::testing::Test
{
protected:
    Template() {}

    void SetUp() override
    {
    }

    void TearDown() override {}

    ~Template() {}

    static void TearDownTestSuite() {}
};

TEST_F(Template, ExampleTest)
{
   EXPECT_TRUE(true);
}
