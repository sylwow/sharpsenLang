#pragma once

#include <functional>
#include <string_view>
#include <iostream>
#include <variant>
#include <deque>

#include "Tokens.hpp"

namespace sharpsenLang {
	class PushBackStream;

	class TokensIterator {
		TokensIterator(const TokensIterator&) = delete;
		void operator=(const TokensIterator&) = delete;
	private:
		std::function<Token()> _getNextToken;
		Token _current;
	public:
		TokensIterator(PushBackStream& stream);
		TokensIterator(std::deque<Token>& tokens);
		
		const Token& operator*() const;
		const Token* operator->() const;
		
		TokensIterator& operator++();
		
		explicit operator bool() const;
	};
}

