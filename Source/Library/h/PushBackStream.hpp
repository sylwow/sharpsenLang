#pragma once
#include <stack>
#include <functional>

namespace sharpsenLang
{
	using GetCharacter = std::function<int()>;

	class PushBackStream
	{
	private:
		const GetCharacter &_input;
		std::stack<int> _stack;
		size_t _lineNumber;
		size_t _charIndex;

	public:
		PushBackStream(const GetCharacter *input);

		int operator()();

		void pushBack(int c);

		size_t lineNumber() const;
		size_t charIndex() const;
	};
}
