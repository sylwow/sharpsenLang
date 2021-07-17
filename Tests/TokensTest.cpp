#include <iostream>
#include <thread>
#include <gtest/gtest.h>

#include "Tokens.hpp"
#include "TestHelpers.hpp"
#include "PushBackStream.hpp"

using namespace sharpsenLang;

class TokensTest : public ::testing::Test
{
protected:
   TokensTest() {}

   void SetUp() override
   {
   }

   void TearDown() override {}

   ~TokensTest() {}

   static void TearDownTestSuite() {}

   PushBackStream &makePBMock(std::string input)
   {
      return pb.makePBMock(input);
   }

   PushBackStreamMocker pb;
};

#define TEST_KEYWORD(key, token)                   \
   TEST_F(TokensTest, GetKeyword_##token)          \
   {                                               \
      std::string keyStr = key;                    \
      auto keyword = getKeyword(keyStr);           \
                                                   \
      EXPECT_EQ(ReservedToken::##token, *keyword); \
   }

TEST_KEYWORD("sizeof", kw_sizeof)
TEST_KEYWORD("tostring", kw_tostring)

TEST_KEYWORD("if", kw_if)
TEST_KEYWORD("else", kw_else)
TEST_KEYWORD("elif", kw_elif)

TEST_KEYWORD("switch", kw_switch)
TEST_KEYWORD("case", kw_case)
TEST_KEYWORD("default", kw_default)

TEST_KEYWORD("for", kw_for)
TEST_KEYWORD("while", kw_while)
TEST_KEYWORD("do", kw_do)

TEST_KEYWORD("break", kw_break)
TEST_KEYWORD("continue", kw_continue)
TEST_KEYWORD("return", kw_return)

TEST_KEYWORD("function", kw_function)

TEST_KEYWORD("void", kw_void)
TEST_KEYWORD("number", kw_number)
TEST_KEYWORD("string", kw_string)

TEST_KEYWORD("public", kw_public)

TEST_F(TokensTest, GetKeywordFail)
{
   std::string key = "horse";
   auto keyword = getKeyword(key);

   EXPECT_FALSE(keyword);
}

#define TEST_OPERATOR(key, token)                  \
   TEST_F(TokensTest, GetOperator_##token)         \
   {                                               \
      std::string input = key;                     \
      auto pb = makePBMock(input);                 \
                                                   \
      auto keyword = getOperator(pb);              \
                                                   \
      EXPECT_EQ(ReservedToken::##token, *keyword); \
   }

TEST_OPERATOR("++", inc)
TEST_OPERATOR("--", dec)

TEST_OPERATOR("+", add)
TEST_OPERATOR("-", sub)
TEST_OPERATOR("..", concat)
TEST_OPERATOR("*", mul)
TEST_OPERATOR("/", div)
TEST_OPERATOR("\\", idiv)
TEST_OPERATOR("%", mod)

TEST_OPERATOR("~", bitwise_not)
TEST_OPERATOR("&", bitwise_and)
TEST_OPERATOR("|", bitwise_or)
TEST_OPERATOR("^", bitwise_xor)
TEST_OPERATOR("<<", shiftl)
TEST_OPERATOR(">>", shiftr)

TEST_OPERATOR("=", assign)

TEST_OPERATOR("+=", add_assign)
TEST_OPERATOR("-=", sub_assign)
TEST_OPERATOR("..=", concat_assign)
TEST_OPERATOR("*=", mul_assign)
TEST_OPERATOR("/=", div_assign)
TEST_OPERATOR("\\=", idiv_assign)
TEST_OPERATOR("%=", mod_assign)

TEST_OPERATOR("&=", and_assign)
TEST_OPERATOR("|=", or_assign)
TEST_OPERATOR("^=", xor_assign)
TEST_OPERATOR("<<=", shiftl_assign)
TEST_OPERATOR(">>=", shiftr_assign)

TEST_OPERATOR("!", logical_not)
TEST_OPERATOR("&&", logical_and)
TEST_OPERATOR("||", logical_or)

TEST_OPERATOR("==", eq)
TEST_OPERATOR("!=", ne)
TEST_OPERATOR("<", lt)
TEST_OPERATOR(">", gt)
TEST_OPERATOR("<=", le)
TEST_OPERATOR(">=", ge)

TEST_OPERATOR("?", question)
TEST_OPERATOR(":", colon)

TEST_OPERATOR(",", comma)

TEST_OPERATOR(";", semicolon)

TEST_OPERATOR("(", open_round)
TEST_OPERATOR(")", close_round)

TEST_OPERATOR("{", open_curly)
TEST_OPERATOR("}", close_curly)

TEST_OPERATOR("[", open_square)
TEST_OPERATOR("]", close_square)

TEST_F(TokensTest, GetOperatorFail)
{
   std::string input = "`!~`";
   auto pb = makePBMock(input);

   auto keyword = getOperator(pb);

   EXPECT_FALSE(keyword);
}

TEST_F(TokensTest, TokenContruct)
{
   Token t(ReservedToken::add, 0, 0);

   EXPECT_FALSE(t.isNumber());
   EXPECT_TRUE(t.isReservedToken());
   EXPECT_EQ(ReservedToken::add, t.getReservedToken());
}