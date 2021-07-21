#include <iostream>
#include <thread>
#include <gtest/gtest.h>

#include "PushBackStream.hpp"
#include "TestHelpers.hpp"
#include "Tokenizer.hpp"

using namespace sharpsenLang;

namespace sharpsenLang
{
    bool operator==(const Token &t1, const Token &t2)
    {
        return t1.hasValue(t2.getValue());
    }

    bool operator!=(const Token &t1, const Token &t2)
    {
        return !(t1 == t2);
    }
}

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

    PushBackStream &makePBMock(std::string input)
    {
        return pb.makePBMock(input);
    }
    std::vector<Token> getResult(std::string input)
    {
        auto stream = makePBMock(input);
        std::vector<Token> result;
        TokensIterator it(stream);
        for (; it; ++it)
        {
            result.push_back(*it);
        }
        return result;
    }

    PushBackStreamMocker pb;
};

TEST_F(TokenizerTest, EmptyTest)
{
    auto input = "";

    auto result = getResult(input);

    std::vector<Token> expected;
    EXPECT_EQ(expected, result);
}

TEST_F(TokenizerTest, NumberLine)
{
    auto input = "number g = 13;";

    auto result = getResult(input);

    std::vector<Token> expected{
        {ReservedToken::KwNumber},
        {Identifier{"g"}},
        {ReservedToken::Assign},
        {13.0},
        {ReservedToken::Semicolon},
    };
    EXPECT_EQ(expected, result);
}

TEST_F(TokenizerTest, StringLine)
{
    auto input = "string g = \"hello my friend\";";

    auto result = getResult(input);

    std::vector<Token> expected{
        {ReservedToken::KwString},
        {Identifier{"g"}},
        {ReservedToken::Assign},
        {"hello my friend"},
        {ReservedToken::Semicolon},
    };
    EXPECT_EQ(expected, result);
}

TEST_F(TokenizerTest, DeclareClass)
{
    auto input = "class MyClass { string g };";

    auto result = getResult(input);

    std::vector<Token> expected{
        {ReservedToken::KwClass},
        {Identifier{"MyClass"}},
        {ReservedToken::OpenCurly},
        {ReservedToken::KwString},
        {Identifier{"g"}},
        {ReservedToken::CloseCurly},
        {ReservedToken::Semicolon},
    };
    bool r = expected == result;
    EXPECT_EQ(expected, result);
}

TEST_F(TokenizerTest, MethodCall)
{
    auto input = "string g = instance.callMethod();";

    auto result = getResult(input);

    std::vector<Token> expected{
        {ReservedToken::KwString},
        {Identifier{"g"}},
        {ReservedToken::Assign},
        {Identifier{"instance"}},
        {ReservedToken::Dot},
        {Identifier{"callMethod"}},
        {ReservedToken::OpenRound},
        {ReservedToken::CloseRound},
        {ReservedToken::Semicolon},
    };
    bool r = expected == result;
    EXPECT_EQ(expected, result);
}

TEST_F(TokenizerTest, MultilineVar)
{
    auto input = R"code(
        number tmp = x;
        x = y;
        y = tmp;
        )code";

    auto result = getResult(input);

    std::vector<Token> expected{
        {ReservedToken::KwNumber},
        {Identifier{"tmp"}},
        {ReservedToken::Assign},
        {Identifier{"x"}},
        {ReservedToken::Semicolon},
        {Identifier{"x"}},
        {ReservedToken::Assign},
        {Identifier{"y"}},
        {ReservedToken::Semicolon},
        {Identifier{"y"}},
        {ReservedToken::Assign},
        {Identifier{"tmp"}},
        {ReservedToken::Semicolon},
    };
    EXPECT_EQ(expected, result);
}

TEST_F(TokenizerTest, SimpleFunction)
{
    auto input = R"code(
        function number less(number x, number y) {
	        return x < y;
        }
        )code";

    auto result = getResult(input);

    std::vector<Token> expected{
        {ReservedToken::KwFunction},
        {ReservedToken::KwNumber},
        {Identifier{"less"}},
        {ReservedToken::OpenRound},
        {ReservedToken::KwNumber},
        {Identifier{"x"}},
        {ReservedToken::Comma},
        {ReservedToken::KwNumber},
        {Identifier{"y"}},
        {ReservedToken::CloseRound},
        {ReservedToken::OpenCurly},
        {ReservedToken::KwReturn},
        {Identifier{"x"}},
        {ReservedToken::Lt},
        {Identifier{"y"}},
        {ReservedToken::Semicolon},
        {ReservedToken::CloseCurly},
    };
    EXPECT_EQ(expected, result);
}

TEST_F(TokenizerTest, OneLineComment)
{
    auto input = R"code(
        function number less(number x, number y) {
	        //return x < y;
        }
        )code";

    auto result = getResult(input);

    std::vector<Token> expected{
        {ReservedToken::KwFunction},
        {ReservedToken::KwNumber},
        {Identifier{"less"}},
        {ReservedToken::OpenRound},
        {ReservedToken::KwNumber},
        {Identifier{"x"}},
        {ReservedToken::Comma},
        {ReservedToken::KwNumber},
        {Identifier{"y"}},
        {ReservedToken::CloseRound},
        {ReservedToken::OpenCurly},
        {ReservedToken::CloseCurly},
    };
    EXPECT_EQ(expected, result);
}

TEST_F(TokenizerTest, MultilineComment)
{
    auto input = R"code(
        function number less(number x, number y) {
            /*for (number j = begin; j < end-1; ++j)
                if (comp(arr[j], pivot))
                    swap(&arr[i++], &arr[j]);
            
            swap (&arr[i], &arr[end-1]);

            quicksort(&arr, begin, i, comp);
            quicksort(&arr, i+1, end, comp);*/
        }
        )code";

    auto result = getResult(input);

    std::vector<Token> expected{
        {ReservedToken::KwFunction},
        {ReservedToken::KwNumber},
        {Identifier{"less"}},
        {ReservedToken::OpenRound},
        {ReservedToken::KwNumber},
        {Identifier{"x"}},
        {ReservedToken::Comma},
        {ReservedToken::KwNumber},
        {Identifier{"y"}},
        {ReservedToken::CloseRound},
        {ReservedToken::OpenCurly},
        {ReservedToken::CloseCurly},
    };
    EXPECT_EQ(expected, result);
}

TEST_F(TokenizerTest, MathematicalOperations)
{
    auto input = R"code(
        number x = 90;
        x += ((12+5/13)*8)/15;
        )code";

    auto result = getResult(input);

    std::vector<Token> expected{
        {ReservedToken::KwNumber},
        {Identifier{"x"}},
        {ReservedToken::Assign},
        {90.0},
        {ReservedToken::Semicolon},
        {Identifier{"x"}},
        {ReservedToken::AddAssign},
        {ReservedToken::OpenRound},
        {ReservedToken::OpenRound},
        {12.0},
        {ReservedToken::Add},
        {5.0},
        {ReservedToken::Div},
        {13.0},
        {ReservedToken::CloseRound},
        {ReservedToken::Mul},
        {8.0},
        {ReservedToken::CloseRound},
        {ReservedToken::Div},
        {15.0},
        {ReservedToken::Semicolon},
    };
    EXPECT_EQ(expected, result);
}

TEST_F(TokenizerTest, ComplexFunction)
{
    auto input = R"code(
        public function void main() {
            number[] arr;
            
            for (number i = 0; i < 10; ++i) {
                arr[sizeof(arr)] = rnd(100);
            }
            
            trace(toString(arr));
            
            sort(&arr, less);
            
            trace(toString(arr));
            
            sort(&arr, greater);
            
            trace(toString(arr));
        }
        )code";

    auto result = getResult(input);

    std::vector<Token> expected{
        {ReservedToken::KwPublic},
        {ReservedToken::KwFunction},
        {ReservedToken::KwVoid},
        {Identifier{"main"}},
        {ReservedToken::OpenRound},
        {ReservedToken::CloseRound},
        {ReservedToken::OpenCurly},

        {ReservedToken::KwNumber},
        {ReservedToken::OpenSquare},
        {ReservedToken::CloseSquare},
        {Identifier{"arr"}},
        {ReservedToken::Semicolon},

        {ReservedToken::KwFor},
        {ReservedToken::OpenRound},
        {ReservedToken::KwNumber},
        {Identifier{"i"}},
        {ReservedToken::Assign},
        {0.0},
        {ReservedToken::Semicolon},
        {Identifier{"i"}},
        {ReservedToken::Lt},
        {10.0},
        {ReservedToken::Semicolon},
        {ReservedToken::Inc},
        {Identifier{"i"}},
        {ReservedToken::CloseRound},
        {ReservedToken::OpenCurly},
        {Identifier{"arr"}},
        {ReservedToken::OpenSquare},
        {ReservedToken::KwSizeof},
        {ReservedToken::OpenRound},
        {Identifier{"arr"}},
        {ReservedToken::CloseRound},
        {ReservedToken::CloseSquare},
        {ReservedToken::Assign},
        {Identifier{"rnd"}},
        {ReservedToken::OpenRound},
        {100.0},
        {ReservedToken::CloseRound},
        {ReservedToken::Semicolon},
        {ReservedToken::CloseCurly},

        {Identifier{"trace"}},
        {ReservedToken::OpenRound},
        {ReservedToken::KwToString},
        {ReservedToken::OpenRound},
        {Identifier{"arr"}},
        {ReservedToken::CloseRound},
        {ReservedToken::CloseRound},
        {ReservedToken::Semicolon},

        {Identifier{"sort"}},
        {ReservedToken::OpenRound},
        {ReservedToken::BitwiseAnd},
        {Identifier{"arr"}},
        {ReservedToken::Comma},
        {Identifier{"less"}},
        {ReservedToken::CloseRound},
        {ReservedToken::Semicolon},

        {Identifier{"trace"}},
        {ReservedToken::OpenRound},
        {ReservedToken::KwToString},
        {ReservedToken::OpenRound},
        {Identifier{"arr"}},
        {ReservedToken::CloseRound},
        {ReservedToken::CloseRound},
        {ReservedToken::Semicolon},

        {Identifier{"sort"}},
        {ReservedToken::OpenRound},
        {ReservedToken::BitwiseAnd},
        {Identifier{"arr"}},
        {ReservedToken::Comma},
        {Identifier{"greater"}},
        {ReservedToken::CloseRound},
        {ReservedToken::Semicolon},

        {Identifier{"trace"}},
        {ReservedToken::OpenRound},
        {ReservedToken::KwToString},
        {ReservedToken::OpenRound},
        {Identifier{"arr"}},
        {ReservedToken::CloseRound},
        {ReservedToken::CloseRound},
        {ReservedToken::Semicolon},

        {ReservedToken::CloseCurly},
    };
    EXPECT_EQ(expected, result);
}
