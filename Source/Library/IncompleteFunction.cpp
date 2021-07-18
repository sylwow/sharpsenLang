#include "IncompleteFunction.hpp"
#include "Compiler.hpp"
#include "CompilerContext.hpp"
#include "Errors.hpp"
#include "Tokenizer.hpp"

namespace sharpsenLang {
	function_declaration parse_function_declaration(CompilerContext& ctx, TokensIterator& it) {
		function_declaration ret;
		
		parse_token_value(ctx, it, ReservedToken::KwFunction);
		
		FunctionType ft;
		ft.returnTypeId = parse_type(ctx, it);
		ret.name = parse_declaration_name(ctx, it);
		
		{
			auto _ = ctx.function();
			
			parse_token_value(ctx, it, ReservedToken::OpenRound);
			
			while(!it->hasValue(ReservedToken::CloseRound)) {
				if (!ret.params.empty()) {
					parse_token_value(ctx, it, ReservedToken::Comma);
				}
				
				TypeHandle t = parse_type(ctx, it);
				bool byref = false;
				if (it->hasValue(ReservedToken::BitwiseAnd)) {
					byref = true;
					++it;
				}
				ft.paramTypeId.push_back({t, byref});
				
				if (!it->hasValue(ReservedToken::CloseRound) && !it->hasValue(ReservedToken::Comma)) {
					ret.params.push_back(parse_declaration_name(ctx, it));
				} else {
					ret.params.push_back("@"+std::to_string(ret.params.size()));
				}
			}
			++it;
		}
		
		ret.type_id = ctx.getHandle(ft);
		
		return ret;
	}

	incomplete_function::incomplete_function(CompilerContext& ctx, TokensIterator& it) {
		_decl = parse_function_declaration(ctx, it);
		
		_tokens.push_back(*it);
		
		parse_token_value(ctx, it, ReservedToken::OpenCurly);
		
		int nesting = 1;
		
		while (nesting && !it->isEof()) {
			if (it->hasValue(ReservedToken::OpenCurly)) {
				++nesting;
			}
			
			if (it->hasValue(ReservedToken::CloseCurly)) {
				--nesting;
			}
			
			_tokens.push_back(*it);
			++it;
		}
		
		if (nesting) {
			throw unexpectedSyntaxError("end of file", it->getLineNumber(), it->getCharIndex());
		}
		
		ctx.createFunction(_decl.name, _decl.type_id);
	}
	
	incomplete_function::incomplete_function(incomplete_function&& orig) noexcept:
		_tokens(std::move(orig._tokens)),
		_decl(std::move(orig._decl))
	{
	}
	
	const function_declaration& incomplete_function::get_decl() const {
		return _decl;
	}
	
	Function incomplete_function::compile(CompilerContext& ctx) {
		auto _ = ctx.function();
		
		const FunctionType* ft = std::get_if<FunctionType>(_decl.type_id);
		
		for (int i = 0; i < int(_decl.params.size()); ++i) {
			ctx.createParam(std::move(_decl.params[i]), ft->paramTypeId[i].typeId);
		}
		
		TokensIterator it(_tokens);
		
		shared_statement_ptr stmt = compile_function_block(ctx, it, ft->returnTypeId);
		
		return [stmt=std::move(stmt)] (RuntimeContext& ctx) {
			stmt->execute(ctx);
		};
	}
}
