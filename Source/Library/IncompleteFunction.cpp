#include "IncompleteFunction.hpp"
#include "Compiler.hpp"
#include "CompilerContext.hpp"
#include "Errors.hpp"
#include "Tokenizer.hpp"

namespace sharpsenLang {
	FunctionDeclaration parseFunctionDeclaration(CompilerContext& ctx, TokensIterator& it) {
		FunctionDeclaration ret;
		
		parseTokenValue(ctx, it, ReservedToken::KwFunction);
		
		FunctionType ft;
		ft.returnTypeId = parseType(ctx, it);
		ret.name = parseDeclarationName(ctx, it);
		
		{
			auto _ = ctx.function();
			
			parseTokenValue(ctx, it, ReservedToken::OpenRound);
			
			while(!it->hasValue(ReservedToken::CloseRound)) {
				if (!ret.params.empty()) {
					parseTokenValue(ctx, it, ReservedToken::Comma);
				}
				
				TypeHandle t = parseType(ctx, it);
				bool byref = false;
				if (it->hasValue(ReservedToken::BitwiseAnd)) {
					byref = true;
					++it;
				}
				ft.paramTypeId.push_back({t, byref});
				
				if (!it->hasValue(ReservedToken::CloseRound) && !it->hasValue(ReservedToken::Comma)) {
					ret.params.push_back(parseDeclarationName(ctx, it));
				} else {
					ret.params.push_back("@"+std::to_string(ret.params.size()));
				}
			}
			++it;
		}
		
		ret.typeId = ctx.getHandle(ft);
		
		return ret;
	}

	IncompleteFunction::IncompleteFunction(CompilerContext& ctx, TokensIterator& it, ClassType* parentClass) {
		this->parentClass = parentClass;
		_decl = parseFunctionDeclaration(ctx, it);
		
		_tokens.push_back(*it);
		
		parseTokenValue(ctx, it, ReservedToken::OpenCurly);
		
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
		
		if(isMethod()) {
			_decl.name = parentClass->fullName + "::" + _decl.name;
		} 
		ctx.createFunction(_decl.name, _decl.typeId);
	}
	
	IncompleteFunction::IncompleteFunction(IncompleteFunction&& orig) noexcept:
		_tokens(std::move(orig._tokens)),
		_decl(std::move(orig._decl))
	{
	}
	
	const FunctionDeclaration& IncompleteFunction::getDecl() const {
		return _decl;
	}
	
	Function IncompleteFunction::compile(CompilerContext& ctx) {
		auto _ = ctx.function();
		
		const FunctionType* ft = std::get_if<FunctionType>(_decl.typeId);
		
		for (int i = 0; i < int(_decl.params.size()); ++i) {
			ctx.createParam(std::move(_decl.params[i]), ft->paramTypeId[i].typeId);
		}
		
		TokensIterator it(_tokens);
		
		SharedStatementPtr stmt = compileFunctionBlock(ctx, it, ft->returnTypeId);
		
		return [stmt=std::move(stmt)] (RuntimeContext& ctx) {
			stmt->execute(ctx);
		};
	}
}
