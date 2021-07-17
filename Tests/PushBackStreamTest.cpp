#include <iostream>
#include <thread>
#include <gtest/gtest.h>

#include "TestHelpers.hpp"
#include "PushBackStream.hpp"

using namespace sharpsenLang;
class PushBackStreamTest : public ::testing::Test
{
protected:
    PushBackStreamTest()
    {
        makePBMock("var gg = 12");
    }

    void SetUp() override
    {
    }

    void TearDown() override {}

    ~PushBackStreamTest() {}

    static void TearDownTestSuite() {}

    PushBackStream &makePBMock(std::string input)
    {
        return pb.makePBMock(input);
    }


    PushBackStreamMocker pb;
};

TEST_F(PushBackStreamTest, ReadStreamTest)
{
    std::string input =
        R"raw(Lorem Ipsum is simply dummy text of the printing and typesetting industry. 
   Lorem Ipsum has been the industry's standard dummy text ever since the 1500s, 
   when an unknown printer took a galley of type and scrambled it to make a type specimen book. 
   It has survived not only five centuries, but also the leap into electronic typesetting, remaining essentially unchanged. 
   It was popularised in the 1960s with the release of Letraset sheets containing Lorem Ipsum passages, 
   and more recently with desktop publishing software like Aldus PageMaker including versions of Lorem Ipsum)raw";

    auto &pb = makePBMock(input);

    std::string output;
    while (true)
    {
        int character = pb();
        if (character == -1)
            break;
        output += character;
    }

    std::string expected = input;

    EXPECT_EQ(expected, output);
    EXPECT_EQ(5, pb.lineNumber());
    EXPECT_EQ(594, pb.charIndex());
}

TEST_F(PushBackStreamTest, WithPushBack)
{
    std::string input = 
    R"raw(Lorem Ipsum is simply dummy text of the printing and typesetting industry. 
   Lorem Ipsum has been the industry's standard dummy text ever since the 1500s, 
   when an unknown printer took a galley of type and scrambled it to make a type specimen book. 
   It has survived not only five centuries, but also the leap into electronic typesetting, remaining essentially unchanged. 
   It was popularised in the 1960s with the release of Letraset sheets containing Lorem Ipsum passages, 
   and more recently with desktop publishing software like Aldus PageMaker including versions of Lorem Ipsum)raw";

    auto &pb = makePBMock(input);

    while (true)
    {
        int character = pb();
        pb.pushBack(character);

        int expected = character;
        EXPECT_EQ(expected, pb());

        if (character == -1)
            break;
    }
}
