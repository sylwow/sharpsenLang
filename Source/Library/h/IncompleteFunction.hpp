#pragma once
#include <deque>
#include <functional>

#include "Tokens.hpp"
#include "Types.hpp"

namespace sharpsenLang {
	class CompilerContext;
	class RuntimeContext;
	class TokensIterator;
	using function = std::function<void(RuntimeContext&)>;

	struct function_declaration{
		std::string name;
		TypeHandle type_id;
		std::vector<std::string> params;
	};
	
	function_declaration parse_function_declaration(CompilerContext& ctx, TokensIterator& it);

	class incomplete_function {
	private:
		function_declaration _decl;
		std::deque<Token> _tokens;
		size_t _index;
	public:
		incomplete_function(CompilerContext& ctx, TokensIterator& it);
		
		incomplete_function(incomplete_function&& orig) noexcept;
		
		const function_declaration& get_decl() const;
		
		function compile(CompilerContext& ctx);
	};
}
