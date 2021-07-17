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

    std::vector<Token> expected{};
    EXPECT_EQ(expected, result);
}

TEST_F(TokenizerTest, VarLine)
{
    auto input = "var g = 13";

    auto result = getResult(input);

    std::vector<Token> expected{
        {Identifier{"var"}},
        {Identifier{"g"}},
        {ReservedToken::assign},
        {13.0},
    };
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
        {ReservedToken::kw_number},
        {Identifier{"tmp"}},
        {ReservedToken::assign},
        {Identifier{"x"}},
        {ReservedToken::semicolon},
        {Identifier{"x"}},
        {ReservedToken::assign},
        {Identifier{"y"}},
        {ReservedToken::semicolon},
        {Identifier{"y"}},
        {ReservedToken::assign},
        {Identifier{"tmp"}},
        {ReservedToken::semicolon},
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
        {ReservedToken::kw_function},
        {ReservedToken::kw_number},
        {Identifier{"less"}},
        {ReservedToken::open_round},
        {ReservedToken::kw_number},
        {Identifier{"x"}},
        {ReservedToken::comma},
        {ReservedToken::kw_number},
        {Identifier{"y"}},
        {ReservedToken::close_round},
        {ReservedToken::open_curly},
        {ReservedToken::kw_return},
        {Identifier{"x"}},
        {ReservedToken::lt},
        {Identifier{"y"}},
        {ReservedToken::semicolon},
        {ReservedToken::close_curly},
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
        {ReservedToken::kw_function},
        {ReservedToken::kw_number},
        {Identifier{"less"}},
        {ReservedToken::open_round},
        {ReservedToken::kw_number},
        {Identifier{"x"}},
        {ReservedToken::comma},
        {ReservedToken::kw_number},
        {Identifier{"y"}},
        {ReservedToken::close_round},
        {ReservedToken::open_curly},
        {ReservedToken::close_curly},
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
        {ReservedToken::kw_function},
        {ReservedToken::kw_number},
        {Identifier{"less"}},
        {ReservedToken::open_round},
        {ReservedToken::kw_number},
        {Identifier{"x"}},
        {ReservedToken::comma},
        {ReservedToken::kw_number},
        {Identifier{"y"}},
        {ReservedToken::close_round},
        {ReservedToken::open_curly},
        {ReservedToken::close_curly},
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
            
            trace(tostring(arr));
            
            sort(&arr, less);
            
            trace(tostring(arr));
            
            sort(&arr, greater);
            
            trace(tostring(arr));
        }
        )code";

    auto result = getResult(input);

    std::vector<Token> expected{
        {ReservedToken::kw_public},
        {ReservedToken::kw_function},
        {ReservedToken::kw_void},
        {Identifier{"main"}},
        {ReservedToken::open_round},
        {ReservedToken::close_round},
        {ReservedToken::open_curly},

        {ReservedToken::kw_number},
        {ReservedToken::open_square},
        {ReservedToken::close_square},
        {Identifier{"arr"}},
        {ReservedToken::semicolon},

        {ReservedToken::kw_for},
        {ReservedToken::open_round},
        {ReservedToken::kw_number},
        {Identifier{"i"}},
        {ReservedToken::assign},
        {0.0},
        {ReservedToken::semicolon},
        {Identifier{"i"}},
        {ReservedToken::lt},
        {10.0},
        {ReservedToken::semicolon},
        {ReservedToken::inc},
        {Identifier{"i"}},
        {ReservedToken::close_round},
        {ReservedToken::open_curly},
        {Identifier{"arr"}},
        {ReservedToken::open_square},
        {ReservedToken::kw_sizeof},
        {ReservedToken::open_round},
        {Identifier{"arr"}},
        {ReservedToken::close_round},
        {ReservedToken::close_square},
        {ReservedToken::assign},
        {Identifier{"rnd"}},
        {ReservedToken::open_round},
        {100.0},
        {ReservedToken::close_round},
        {ReservedToken::semicolon},
        {ReservedToken::close_curly},

        {Identifier{"trace"}},
        {ReservedToken::open_round},
        {ReservedToken::kw_tostring},
        {ReservedToken::open_round},
        {Identifier{"arr"}},
        {ReservedToken::close_round},
        {ReservedToken::close_round},
        {ReservedToken::semicolon},

        {Identifier{"sort"}},
        {ReservedToken::open_round},
        {ReservedToken::bitwise_and},
        {Identifier{"arr"}},
        {ReservedToken::comma},
        {Identifier{"less"}},
        {ReservedToken::close_round},
        {ReservedToken::semicolon},

        {Identifier{"trace"}},
        {ReservedToken::open_round},
        {ReservedToken::kw_tostring},
        {ReservedToken::open_round},
        {Identifier{"arr"}},
        {ReservedToken::close_round},
        {ReservedToken::close_round},
        {ReservedToken::semicolon},

        {Identifier{"sort"}},
        {ReservedToken::open_round},
        {ReservedToken::bitwise_and},
        {Identifier{"arr"}},
        {ReservedToken::comma},
        {Identifier{"greater"}},
        {ReservedToken::close_round},
        {ReservedToken::semicolon},

        {Identifier{"trace"}},
        {ReservedToken::open_round},
        {ReservedToken::kw_tostring},
        {ReservedToken::open_round},
        {Identifier{"arr"}},
        {ReservedToken::close_round},
        {ReservedToken::close_round},
        {ReservedToken::semicolon},

        {ReservedToken::close_curly},
    };
    EXPECT_EQ(expected, result);
}
