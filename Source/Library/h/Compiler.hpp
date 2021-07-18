#pragma once
#include <vector>
#include <functional>

#include "Types.hpp"
#include "Tokens.hpp"
#include "Statement.hpp"

namespace sharpsenLang
{
	class CompilerContext;
	class TokensIterator;
	class RuntimeContext;

	using Function = std::function<void(RuntimeContext &)>;

	RuntimeContext compile(
		TokensIterator &it,
		const std::vector<std::pair<std::string, Function>> &externalFunctions,
		std::vector<std::string> publicDeclarations);

	TypeHandle parseType(CompilerContext &ctx, TokensIterator &it);

	std::string parseDeclarationName(CompilerContext &ctx, TokensIterator &it);

	void parseTokenValue(CompilerContext &ctx, TokensIterator &it, const TokenValue &value);

	SharedStatementPtr compileFunctionBlock(CompilerContext &ctx, TokensIterator &it, TypeHandle returnTypeId);
}
