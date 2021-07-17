#pragma once

#include <optional>
#include <string_view>
#include <ostream>
#include <variant>

namespace sharpsenLang
{
	enum struct ReservedToken
	{
		inc,
		dec,

		add,
		sub,
		concat,
		mul,
		div,
		idiv,
		mod,

		bitwise_not,
		bitwise_and,
		bitwise_or,
		bitwise_xor,
		shiftl,
		shiftr,

		assign,

		add_assign,
		sub_assign,
		concat_assign,
		mul_assign,
		div_assign,
		idiv_assign,
		mod_assign,

		and_assign,
		or_assign,
		xor_assign,
		shiftl_assign,
		shiftr_assign,

		logical_not,
		logical_and,
		logical_or,

		eq,
		ne,
		lt,
		gt,
		le,
		ge,

		question,
		colon,

		comma,

		semicolon,

		open_round,
		close_round,

		open_curly,
		close_curly,

		open_square,
		close_square,

		kw_sizeof,
		kw_tostring,

		kw_if,
		kw_else,
		kw_elif,

		kw_switch,
		kw_case,
		kw_default,

		kw_for,
		kw_while,
		kw_do,

		kw_break,
		kw_continue,
		kw_return,

		kw_function,

		kw_void,
		kw_number,
		kw_string,

		kw_public,
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

	bool operator==(const Token &, const Token &);
	bool operator!=(const Token &, const Token &);
}

namespace std
{
	std::string to_string(sharpsenLang::ReservedToken t);
	std::string to_string(const sharpsenLang::TokenValue &t);
}