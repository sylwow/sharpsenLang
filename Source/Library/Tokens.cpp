#include <stack>
#include <string_view>

#include "Tokens.hpp"
#include "Lookup.hpp"
#include "Helpers.hpp"
#include "PushBackStream.hpp"

namespace sharpsenLang
{
	namespace
	{
		const Lookup<std::string_view, ReservedToken> operatorTokenMap{
			{"++", ReservedToken::inc},
			{"--", ReservedToken::dec},

			{"+", ReservedToken::add},
			{"-", ReservedToken::sub},
			{"..", ReservedToken::concat},
			{"*", ReservedToken::mul},
			{"/", ReservedToken::div},
			{"\\", ReservedToken::idiv},
			{"%", ReservedToken::mod},

			{"~", ReservedToken::bitwise_not},
			{"&", ReservedToken::bitwise_and},
			{"|", ReservedToken::bitwise_or},
			{"^", ReservedToken::bitwise_xor},
			{"<<", ReservedToken::shiftl},
			{">>", ReservedToken::shiftr},

			{"=", ReservedToken::assign},

			{"+=", ReservedToken::add_assign},
			{"-=", ReservedToken::sub_assign},
			{"..=", ReservedToken::concat_assign},
			{"*=", ReservedToken::mul_assign},
			{"/=", ReservedToken::div_assign},
			{"\\=", ReservedToken::idiv_assign},
			{"%=", ReservedToken::mod_assign},

			{"&=", ReservedToken::and_assign},
			{"|=", ReservedToken::or_assign},
			{"^=", ReservedToken::xor_assign},
			{"<<=", ReservedToken::shiftl_assign},
			{">>=", ReservedToken::shiftr_assign},

			{"!", ReservedToken::logical_not},
			{"&&", ReservedToken::logical_and},
			{"||", ReservedToken::logical_or},

			{"==", ReservedToken::eq},
			{"!=", ReservedToken::ne},
			{"<", ReservedToken::lt},
			{">", ReservedToken::gt},
			{"<=", ReservedToken::le},
			{">=", ReservedToken::ge},

			{"?", ReservedToken::question},
			{":", ReservedToken::colon},

			{",", ReservedToken::comma},

			{";", ReservedToken::semicolon},

			{"(", ReservedToken::open_round},
			{")", ReservedToken::close_round},

			{"{", ReservedToken::open_curly},
			{"}", ReservedToken::close_curly},

			{"[", ReservedToken::open_square},
			{"]", ReservedToken::close_square},
		};

		const Lookup<std::string_view, ReservedToken> keywordTokenMap{
			{"sizeof", ReservedToken::kw_sizeof},
			{"tostring", ReservedToken::kw_tostring},

			{"if", ReservedToken::kw_if},
			{"else", ReservedToken::kw_else},
			{"elif", ReservedToken::kw_elif},

			{"switch", ReservedToken::kw_switch},
			{"case", ReservedToken::kw_case},
			{"default", ReservedToken::kw_default},

			{"for", ReservedToken::kw_for},
			{"while", ReservedToken::kw_while},
			{"do", ReservedToken::kw_do},

			{"break", ReservedToken::kw_break},
			{"continue", ReservedToken::kw_continue},
			{"return", ReservedToken::kw_return},

			{"function", ReservedToken::kw_function},

			{"void", ReservedToken::kw_void},
			{"number", ReservedToken::kw_number},
			{"string", ReservedToken::kw_string},

			{"public", ReservedToken::kw_public}};

		const Lookup<ReservedToken, std::string_view> tokenStringMap = ([]()
																		  {
																			  std::vector<std::pair<ReservedToken, std::string_view>> container;
																			  container.reserve(operatorTokenMap.size() + keywordTokenMap.size());
																			  for (const auto &p : operatorTokenMap)
																			  {
																				  container.emplace_back(p.second, p.first);
																			  }
																			  for (const auto &p : keywordTokenMap)
																			  {
																				  container.emplace_back(p.second, p.first);
																			  }
																			  return Lookup<ReservedToken, std::string_view>(std::move(container));
																		  })();
	}

	std::optional<ReservedToken> getKeyword(std::string_view word)
	{
		auto it = keywordTokenMap.find(word);
		return it == keywordTokenMap.end() ? std::nullopt : std::make_optional(it->second);
	}

	namespace
	{
		class MaximalMunchComparator
		{
		private:
			size_t _idx;

		public:
			MaximalMunchComparator(size_t idx) : _idx(idx)
			{
			}

			bool operator()(char l, char r) const
			{
				return l < r;
			}

			bool operator()(std::pair<std::string_view, ReservedToken> l, char r) const
			{
				return l.first.size() <= _idx || l.first[_idx] < r;
			}

			bool operator()(char l, std::pair<std::string_view, ReservedToken> r) const
			{
				return r.first.size() > _idx && l < r.first[_idx];
			}

			bool operator()(std::pair<std::string_view, ReservedToken> l, std::pair<std::string_view, ReservedToken> r) const
			{
				return r.first.size() > _idx && (l.first.size() < _idx || l.first[_idx] < r.first[_idx]);
			}
		};
	}

	std::optional<ReservedToken> getOperator(PushBackStream &stream)
	{
		auto candidates = std::make_pair(operatorTokenMap.begin(), operatorTokenMap.end());

		std::optional<ReservedToken> ret;
		size_t match_size = 0;

		std::stack<int> chars;

		for (size_t idx = 0; candidates.first != candidates.second; ++idx)
		{
			chars.push(stream());

			candidates = std::equal_range(candidates.first, candidates.second, char(chars.top()), MaximalMunchComparator(idx));

			if (candidates.first != candidates.second && candidates.first->first.size() == idx + 1)
			{
				match_size = idx + 1;
				ret = candidates.first->second;
			}
		}

		while (chars.size() > match_size)
		{
			stream.pushBack(chars.top());
			chars.pop();
		}

		return ret;
	}

	Token::Token(TokenValue value, size_t lineNumber, size_t charIndex) : _value(std::move(value)),
																			_lineNumber(lineNumber),
																			_charIndex(charIndex)
	{
	}

	bool Token::isReservedToken() const
	{
		return std::holds_alternative<ReservedToken>(_value);
	}

	bool Token::isIdentifier() const
	{
		return std::holds_alternative<Identifier>(_value);
	}

	bool Token::isNumber() const
	{
		return std::holds_alternative<double>(_value);
	}

	bool Token::isString() const
	{
		return std::holds_alternative<std::string>(_value);
	}

	bool Token::isEof() const
	{
		return std::holds_alternative<Eof>(_value);
	}

	ReservedToken Token::getReservedToken() const
	{
		return std::get<ReservedToken>(_value);
	}

	const Identifier &Token::getIdentifier() const
	{
		return std::get<Identifier>(_value);
	}

	double Token::getNumber() const
	{
		return std::get<double>(_value);
	}

	const std::string &Token::getString() const
	{
		return std::get<std::string>(_value);
	}

	const TokenValue &Token::getValue() const
	{
		return _value;
	}

	size_t Token::getLineNumber() const
	{
		return _lineNumber;
	}

	size_t Token::getCharIndex() const
	{
		return _charIndex;
	}

	bool Token::hasValue(const TokenValue &value) const
	{
		return _value == value;
	}

	bool operator==(const Identifier &id1, const Identifier &id2)
	{
		return id1.name == id2.name;
	}

	bool operator!=(const Identifier &id1, const Identifier &id2)
	{
		return id1.name != id2.name;
	}

	bool operator==(const Eof &, const Eof &)
	{
		return true;
	}

	bool operator!=(const Eof &, const Eof &)
	{
		return false;
	}
}

namespace std
{
	using namespace sharpsenLang;
	std::string to_string(ReservedToken t)
	{
		return std::string(tokenStringMap.find(t)->second);
	}

	std::string to_string(const TokenValue &t)
	{
		return std::visit(overloaded{[](ReservedToken rt)
									 {
										 return to_string(rt);
									 },
									 [](double d)
									 {
										 return to_string(d);
									 },
									 [](const std::string &str)
									 {
										 return str;
									 },
									 [](const Identifier &id)
									 {
										 return id.name;
									 },
									 [](Eof)
									 {
										 return std::string("<EOF>");
									 }},
						  t);
	}
}
