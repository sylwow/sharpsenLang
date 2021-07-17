#pragma once

#include "PushBackStream.hpp"

namespace sharpsenLang
{
	class PushBackStreamMocker
	{
	public:
		PushBackStream &makePBMock(std::string input)
		{

			array = std::vector<int>(input.begin(), input.end());
			array.push_back(-1);
			index = 0;
			getChar = [this]()
			{
				return array.at(index++);
			};
			pushBackStream = std::make_unique<PushBackStream>(&getChar);
			return getPB();
		}

		PushBackStream &getPB()
		{
			return *pushBackStream;
		}

	private:
		int index;
		std::vector<int> array;
		GetCharacter getChar;
		std::unique_ptr<PushBackStream> pushBackStream;
	};
}
