#pragma once
#include <deque>
#include <functional>

#include "Tokens.hpp"
#include "Types.hpp"
#include "IncompleteFunction.hpp"
#include "Variable.hpp"

namespace sharpsenLang
{
	class CompilerContext;
	class RuntimeContext;
	class TokensIterator;
	using Function = std::function<void(RuntimeContext &)>;

	struct ClassDeclaration
	{
		std::string name;
		TypeHandle typeId;
		std::vector<std::string> properties;
		std::vector<FunctionDeclaration> methods;
	};


	class IncompleteClass
	{
	private:
		ClassDeclaration _decl;
		std::deque<Token> _tokens;
		size_t _index;
		std::vector<IncompleteFunction> _incompleteMethods;

	public:
		IncompleteClass(CompilerContext &ctx, TokensIterator &it);

		IncompleteClass(IncompleteClass &&orig) noexcept;

		const ClassDeclaration &getDecl() const;

		Class compile(CompilerContext &ctx);

		std::vector<IncompleteFunction>& getIncompletedMethods();
	
	private:
		ClassDeclaration parseClassDeclaration(CompilerContext &ctx, TokensIterator &it);
	};
}
