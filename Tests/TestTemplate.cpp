#include <iostream>
#include <thread>
#include <gtest/gtest.h>

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
