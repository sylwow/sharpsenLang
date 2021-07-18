#pragma once
#include <string>

#include "Variable.hpp"
#include "Types.hpp"


namespace sharpsenLang
{
	class RuntimeContext;
	class TokensIterator;
	class CompilerContext;

	template <typename R>
	class Expression
	{
		Expression(const Expression &) = delete;
		void operator=(const Expression &) = delete;

	protected:
		Expression() = default;

	public:
		using Ptr = std::unique_ptr<const Expression>;

		virtual R evaluate(RuntimeContext &context) const = 0;
		virtual ~Expression() = default;
	};

	Expression<void>::Ptr buildVoidExpression(CompilerContext &context, TokensIterator &it);
	Expression<Number>::Ptr buildNumberExpression(CompilerContext &context, TokensIterator &it);
	Expression<Lvalue>::Ptr buildInitializationExpression(
		CompilerContext &context,
		TokensIterator &it,
		TypeHandle typeId,
		bool allow_comma);
	Expression<Lvalue>::Ptr buildDefaultInitialization(TypeHandle typeId);
}
