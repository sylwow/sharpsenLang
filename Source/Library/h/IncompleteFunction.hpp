#pragma once
#include <deque>
#include <functional>

#include "Tokens.hpp"
#include "Types.hpp"

namespace sharpsenLang
{
	class CompilerContext;
	class RuntimeContext;
	class TokensIterator;
	using Function = std::function<void(RuntimeContext &)>;

	struct FunctionDeclaration
	{
		std::string name;
		TypeHandle typeId;
		std::vector<std::string> params;
	};

	FunctionDeclaration parseFunctionDeclaration(CompilerContext &ctx, TokensIterator &it);

	class IncompleteFunction
	{
	private:
		FunctionDeclaration _decl;
		std::deque<Token> _tokens;
		size_t _index;

	public:
		IncompleteFunction(CompilerContext &ctx, TokensIterator &it);

		IncompleteFunction(IncompleteFunction &&orig) noexcept;

		const FunctionDeclaration &getDecl() const;

		Function compile(CompilerContext &ctx);
	};
}
