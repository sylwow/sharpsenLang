#pragma once
#include <memory>

#include "Types.hpp"

namespace sharpsenLang
{
	struct Node;
	using NodePtr = std::unique_ptr<Node>;

	class TokensIterator;

	class CompilerContext;

	NodePtr parseExpressionTree(CompilerContext &context, TokensIterator &it, TypeHandle type_id, bool allow_comma);
}
