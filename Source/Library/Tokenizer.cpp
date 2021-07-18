#include <map>
#include <string>
#include <cctype>
#include <stack>
#include <cstdlib>

#include "Tokenizer.hpp"
#include "PushBackStream.hpp"
#include "Errors.hpp"

namespace sharpsenLang
{
	namespace
	{
		enum struct CharacterType
		{
			Eof,
			Space,
			Alphanum,
			Punct,
		};

		CharacterType getCharacterType(int c)
		{
			if (c < 0)
			{
				return CharacterType::Eof;
			}
			if (std::isspace(c))
			{
				return CharacterType::Space;
			}
			if (std::isalpha(c) || std::isdigit(c) || c == '_')
			{
				return CharacterType::Alphanum;
			}
			return CharacterType::Punct;
		}

		Token fetchWord(PushBackStream &stream)
		{
			size_t lineNumber = stream.lineNumber();
			size_t charIndex = stream.charIndex();

			std::string word;

			int c = stream();

			bool isNumber = isdigit(c);

			do
			{
				word.push_back(char(c));
				c = stream();

				if (c == '.' && word.back() == '.')
				{
					stream.pushBack(word.back());
					word.pop_back();
					break;
				}
			} while (getCharacterType(c) == CharacterType::Alphanum || (isNumber && c == '.'));

			stream.pushBack(c);

			if (std::optional<ReservedToken> t = getKeyword(word))
			{
				return Token(*t, lineNumber, charIndex);
			}
			else
			{
				if (std::isdigit(word.front()))
				{
					char *endptr;
					double num = strtol(word.c_str(), &endptr, 0);
					if (*endptr != 0)
					{
						num = strtod(word.c_str(), &endptr);
						if (*endptr != 0)
						{
							size_t remaining = word.size() - (endptr - word.c_str());
							throw unexpectedError(
								std::string(1, char(*endptr)),
								stream.lineNumber(),
								stream.charIndex() - remaining);
						}
					}
					return Token(num, lineNumber, charIndex);
				}
				else
				{
					return Token(Identifier{std::move(word)}, lineNumber, charIndex);
				}
			}
		}

		Token fetchOperator(PushBackStream &stream)
		{
			size_t lineNumber = stream.lineNumber();
			size_t charIndex = stream.charIndex();

			if (std::optional<ReservedToken> t = getOperator(stream))
			{
				return Token(*t, lineNumber, charIndex);
			}
			else
			{
				std::string unexpected;
				size_t err_line_number = stream.lineNumber();
				size_t err_char_index = stream.charIndex();
				for (int c = stream(); getCharacterType(c) == CharacterType::Punct; c = stream())
				{
					unexpected.push_back(char(c));
				}
				throw unexpectedError(unexpected, err_line_number, err_char_index);
			}
		}

		Token fetchString(PushBackStream &stream)
		{
			size_t lineNumber = stream.lineNumber();
			size_t charIndex = stream.charIndex();

			std::string str;

			bool escaped = false;
			int c = stream();
			for (; getCharacterType(c) != CharacterType::Eof; c = stream())
			{
				if (c == '\\')
				{
					escaped = true;
				}
				else
				{
					if (escaped)
					{
						switch (c)
						{
						case 't':
							str.push_back('\t');
							break;
						case 'n':
							str.push_back('\n');
							break;
						case 'r':
							str.push_back('\r');
							break;
						case '0':
							str.push_back('\0');
							break;
						default:
							str.push_back(c);
							break;
						}
						escaped = false;
					}
					else
					{
						switch (c)
						{
						case '\t':
						case '\n':
						case '\r':
							stream.pushBack(c);
							throw parsingError("Expected closing '\"'", stream.lineNumber(), stream.charIndex());
						case '"':
							return Token(std::move(str), lineNumber, charIndex);
						default:
							str.push_back(c);
						}
					}
				}
			}
			stream.pushBack(c);
			throw parsingError("Expected closing '\"'", stream.lineNumber(), stream.charIndex());
		}

		void skipLineComment(PushBackStream &stream)
		{
			int c;
			do
			{
				c = stream();
			} while (c != '\n' && getCharacterType(c) != CharacterType::Eof);

			if (c != '\n')
			{
				stream.pushBack(c);
			}
		}

		void skipBlockComment(PushBackStream &stream)
		{
			bool closing = false;
			int c;
			do
			{
				c = stream();
				if (closing && c == '/')
				{
					return;
				}
				closing = (c == '*');
			} while (getCharacterType(c) != CharacterType::Eof);

			stream.pushBack(c);
			throw parsingError("Expected closing '*/'", stream.lineNumber(), stream.charIndex());
		}

		Token tokenize(PushBackStream &stream)
		{
			while (true)
			{
				size_t lineNumber = stream.lineNumber();
				size_t charIndex = stream.charIndex();
				int c = stream();
				switch (getCharacterType(c))
				{
				case CharacterType::Eof:
					return {Eof(), lineNumber, charIndex};
				case CharacterType::Space:
					continue;
				case CharacterType::Alphanum:
					stream.pushBack(c);
					return fetchWord(stream);
				case CharacterType::Punct:
					switch (c)
					{
					case '"':
						return fetchString(stream);
					case '/':
					{
						char c1 = stream();
						switch (c1)
						{
						case '/':
							skipLineComment(stream);
							continue;
						case '*':
							skipBlockComment(stream);
							continue;
						default:
							stream.pushBack(c1);
						}
					}
					default:
						stream.pushBack(c);
						return fetchOperator(stream);
					}
					break;
				}
			}
		}
	}

	TokensIterator::TokensIterator(PushBackStream &stream)
		: _current(Eof(), 0, 0),
		  _getNextToken([&stream]()
						{ return tokenize(stream); })
	{
		++(*this);
	}

	TokensIterator::TokensIterator(std::deque<Token> &tokens)
		: _current(Eof(), 0, 0),
		  _getNextToken([&tokens]()
						{
							if (tokens.empty())
							{
								return Token(Eof(), 0, 0);
							}
							else
							{
								Token ret = std::move(tokens.front());
								tokens.pop_front();
								return ret;
							}
						})
	{
		++(*this);
	}

	TokensIterator &TokensIterator::operator++()
	{
		_current = _getNextToken();
		return *this;
	}

	const Token &TokensIterator::operator*() const
	{
		return _current;
	}

	const Token *TokensIterator::operator->() const
	{
		return &_current;
	}

	TokensIterator::operator bool() const
	{
		return !_current.isEof();
	}
}
