#include "PushBackStream.hpp"

namespace sharpsenLang
{
	PushBackStream::PushBackStream(const GetCharacter *input) : _input(*input),
																_lineNumber(0),
																_charIndex(0)
	{
	}

	int PushBackStream::operator()()
	{
		int ret = -1;
		if (_stack.empty())
		{
			ret = _input();
		}
		else
		{
			ret = _stack.top();
			_stack.pop();
		}
		if (ret == '\n')
		{
			++_lineNumber;
		}

		++_charIndex;

		return ret;
	}

	void PushBackStream::pushBack(int c)
	{
		_stack.push(c);

		if (c == '\n')
		{
			--_lineNumber;
		}

		--_charIndex;
	}

	size_t PushBackStream::lineNumber() const
	{
		return _lineNumber;
	}

	size_t PushBackStream::charIndex() const
	{
		return _charIndex;
	}
}
