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
		TypeHandle parentTypeId = nullptr;
		bool isMethod() {
			return parentTypeId;
		}
	};

	FunctionDeclaration parseFunctionDeclaration(CompilerContext &ctx, TokensIterator &it);

	class IncompleteFunction
	{
	private:
		ClassType *parentClass = nullptr;
		FunctionDeclaration _decl;
		size_t _index;
		std::deque<Token> _tokens;

	public:
		IncompleteFunction(CompilerContext &ctx, TokensIterator &it, ClassType *parentClass = nullptr);

		IncompleteFunction(IncompleteFunction &&orig) noexcept;

		void updateParentClass(TypeHandle parentClass);

		const FunctionDeclaration &getDecl() const;

		Function compile(CompilerContext &ctx);

	private:
		bool isMethod()
		{
			return parentClass != nullptr;
		}
	};
}
