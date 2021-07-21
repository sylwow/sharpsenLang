#pragma once
#include <optional>
#include <string_view>
#include <ostream>
#include <variant>

namespace sharpsenLang
{
	enum struct ReservedToken
	{
		Inc,
		Dec,

		Add,
		Sub,
		Concat,
		Mul,
		Div,
		Idiv,
		Mod,

		BitwiseNot,
		BitwiseAnd,
		BitwiseOr,
		BitwiseXor,
		Shiftl,
		Shiftr,

		Assign,

		Dot,

		AddAssign,
		SubAssign,
		ConcatAssign,
		MulAssign,
		DivAssign,
		IdivAssign,
		ModAssign,

		AndAssign,
		OrAssign,
		XorAssign,
		ShiftlAssign,
		ShiftrAssign,

		LogicalNot,
		LogicalAnd,
		LogicalOr,

		Eq,
		Ne,
		Lt,
		Gt,
		Le,
		Ge,

		Question,
		Colon,

		Comma,

		Semicolon,

		OpenRound,
		CloseRound,

		OpenCurly,
		CloseCurly,

		OpenSquare,
		CloseSquare,

		KwClass, 

		KwSizeof,
		KwToString,

		KwIf,
		KwElse,
		KwElif,

		KwSwitch,
		KwCase,
		KwDefault,

		KwFor,
		KwWhile,
		KwDo,

		KwBreak,
		KwContinue,
		KwReturn,

		KwFunction,

		KwVoid,
		KwNumber,
		KwString,

		KwPublic,
	};

	class PushBackStream;

	std::optional<ReservedToken> getKeyword(std::string_view word);

	std::optional<ReservedToken> getOperator(PushBackStream &stream);

	struct Identifier
	{
		std::string name;
	};

	bool operator==(const Identifier &id1, const Identifier &id2);
	bool operator!=(const Identifier &id1, const Identifier &id2);

	struct Eof
	{
	};

	bool operator==(const Eof &, const Eof &);
	bool operator!=(const Eof &, const Eof &);

	using TokenValue = std::variant<ReservedToken, Identifier, double, std::string, Eof>;

	class Token
	{
	private:
		TokenValue _value;
		size_t _lineNumber;
		size_t _charIndex;

	public:
		Token(TokenValue value, size_t lineNumber = 0, size_t charIndex = 0);

		bool isReservedToken() const;
		bool isIdentifier() const;
		bool isNumber() const;
		bool isString() const;
		bool isEof() const;

		ReservedToken getReservedToken() const;
		const Identifier &getIdentifier() const;
		double getNumber() const;
		const std::string &getString() const;
		const TokenValue &getValue() const;

		size_t getLineNumber() const;
		size_t getCharIndex() const;

		bool hasValue(const TokenValue &value) const;
	};
}

namespace std
{
	std::string to_string(sharpsenLang::ReservedToken t);
	std::string to_string(const sharpsenLang::TokenValue &t);
}