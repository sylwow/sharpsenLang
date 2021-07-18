#pragma once
#include <vector>
#include <functional>

#include "Types.hpp"
#include "Tokens.hpp"
#include "Statement.hpp"

namespace sharpsenLang {
	class CompilerContext;
	class TokensIterator;
	class RuntimeContext;
	
	using function = std::function<void(RuntimeContext&)>;

	RuntimeContext compile(
		TokensIterator& it,
		const std::vector<std::pair<std::string, function> >& external_functions,
		std::vector<std::string> public_declarations
	);
	
	TypeHandle parse_type(CompilerContext& ctx, TokensIterator& it);

	std::string parse_declaration_name(CompilerContext& ctx, TokensIterator& it);
	
	void parse_token_value(CompilerContext& ctx, TokensIterator& it, const TokenValue& value);
	
	shared_statement_ptr compile_function_block(CompilerContext& ctx, TokensIterator& it, TypeHandle return_type_id);
}
