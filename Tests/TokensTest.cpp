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

TEST_KEYWORD("class", KwClass)

TEST_KEYWORD("sizeof", KwSizeof)
TEST_KEYWORD("toString", KwToString)

TEST_KEYWORD("if", KwIf)
TEST_KEYWORD("else", KwElse)
TEST_KEYWORD("elif", KwElif)

TEST_KEYWORD("switch", KwSwitch)
TEST_KEYWORD("case", KwCase)
TEST_KEYWORD("default", KwDefault)

TEST_KEYWORD("for", KwFor)
TEST_KEYWORD("while", KwWhile)
TEST_KEYWORD("do", KwDo)

TEST_KEYWORD("break", KwBreak)
TEST_KEYWORD("continue", KwContinue)
TEST_KEYWORD("return", KwReturn)

TEST_KEYWORD("function", KwFunction)

TEST_KEYWORD("void", KwVoid)
TEST_KEYWORD("number", KwNumber)
TEST_KEYWORD("string", KwString)

TEST_KEYWORD("public", KwPublic)

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

TEST_OPERATOR("++", Inc)
TEST_OPERATOR("--", Dec)

TEST_OPERATOR("+", Add)
TEST_OPERATOR("-", Sub)
TEST_OPERATOR("..", Concat)
TEST_OPERATOR("*", Mul)
TEST_OPERATOR("/", Div)
TEST_OPERATOR("\\", Idiv)
TEST_OPERATOR("%", Mod)

TEST_OPERATOR("~", BitwiseNot)
TEST_OPERATOR("&", BitwiseAnd)
TEST_OPERATOR("|", BitwiseOr)
TEST_OPERATOR("^", BitwiseXor)
TEST_OPERATOR("<<", Shiftl)
TEST_OPERATOR(">>", Shiftr)

TEST_OPERATOR("=", Assign)

TEST_OPERATOR(".", Dot)

TEST_OPERATOR("+=", AddAssign)
TEST_OPERATOR("-=", SubAssign)
TEST_OPERATOR("..=", ConcatAssign)
TEST_OPERATOR("*=", MulAssign)
TEST_OPERATOR("/=", DivAssign)
TEST_OPERATOR("\\=", IdivAssign)
TEST_OPERATOR("%=", ModAssign)

TEST_OPERATOR("&=", AndAssign)
TEST_OPERATOR("|=", OrAssign)
TEST_OPERATOR("^=", XorAssign)
TEST_OPERATOR("<<=", ShiftlAssign)
TEST_OPERATOR(">>=", ShiftrAssign)

TEST_OPERATOR("!", LogicalNot)
TEST_OPERATOR("&&", LogicalAnd)
TEST_OPERATOR("||", LogicalOr)

TEST_OPERATOR("==", Eq)
TEST_OPERATOR("!=", Ne)
TEST_OPERATOR("<", Lt)
TEST_OPERATOR(">", Gt)
TEST_OPERATOR("<=", Le)
TEST_OPERATOR(">=", Ge)

TEST_OPERATOR("?", Question)
TEST_OPERATOR(":", Colon)

TEST_OPERATOR(",", Comma)

TEST_OPERATOR(";", Semicolon)

TEST_OPERATOR("(", OpenRound)
TEST_OPERATOR(")", CloseRound)

TEST_OPERATOR("{", OpenCurly)
TEST_OPERATOR("}", CloseCurly)

TEST_OPERATOR("[", OpenSquare)
TEST_OPERATOR("]", CloseSquare)

TEST_F(TokensTest, GetOperatorFail)
{
   std::string input = "`!~`";
   auto pb = makePBMock(input);

   auto keyword = getOperator(pb);

   EXPECT_FALSE(keyword);
}

TEST_F(TokensTest, TokenContruct)
{
   Token t(ReservedToken::Add, 0, 0);

   EXPECT_FALSE(t.isNumber());
   EXPECT_TRUE(t.isReservedToken());
   EXPECT_EQ(ReservedToken::Add, t.getReservedToken());
}