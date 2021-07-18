#include "Compiler.hpp"
#include "Errors.hpp"
#include "CompilerContext.hpp"
#include "Expression.hpp"
#include "IncompleteFunction.hpp"
#include "Tokenizer.hpp"
#include "RuntimeContext.hpp"
#include "Helpers.hpp"
#include "PushBackStream.hpp"

namespace sharpsenLang {
	namespace {
		struct possible_flow {
			size_t break_level;
			bool can_continue;
			TypeHandle return_type_id;
			
			possible_flow add_switch() {
				return possible_flow{break_level+1, can_continue, return_type_id};
			}
			
			possible_flow add_loop() {
				return possible_flow{break_level+1, true, return_type_id};
			}
			
			static possible_flow in_function(TypeHandle return_type_id) {
				return possible_flow{0, false, return_type_id};
			}
		};
	
		bool is_typename(const CompilerContext&, const TokensIterator& it) {
			return std::visit(overloaded{
				[](ReservedToken t) {
					switch (t) {
						case ReservedToken::KwNumber:
						case ReservedToken::KwString:
						case ReservedToken::KwVoid:
						case ReservedToken::OpenSquare:
							return true;
						default:
							return false;
					}
				},
				[](const TokenValue&) {
					return false;
				}
			}, it->getValue());
		}
	
		Error unexpected_syntax(const TokensIterator& it) {
			return unexpectedSyntaxError(std::to_string(it->getValue()), it->getLineNumber(), it->getCharIndex());
		}
		
		std::vector<Expression<Lvalue>::Ptr> compile_variable_declaration(CompilerContext& ctx, TokensIterator& it) {
			TypeHandle type_id = parse_type(ctx, it);
		
			if (type_id == TypeRegistry::getVoidHandle()) {
				throw syntaxError("Cannot declare void variable", it->getLineNumber(), it->getCharIndex());
			}
			
			std::vector<Expression<Lvalue>::Ptr> ret;
			
			do {
				if (!ret.empty()) {
					++it;
				}
			
				std::string name = parse_declaration_name(ctx, it);
			
				if (it->hasValue(ReservedToken::OpenRound)) {
					++it;
					ret.emplace_back(buildInitializationExpression(ctx, it, type_id, false));
					parse_token_value(ctx, it, ReservedToken::CloseRound);
				} else if (it->hasValue(ReservedToken::Assign)) {
					++it;
					ret.emplace_back(buildInitializationExpression(ctx, it, type_id, false));
				} else {
					ret.emplace_back(buildDefaultInitialization(type_id));
				}
				
				ctx.createIdentifier(std::move(name), type_id);
			} while (it->hasValue(ReservedToken::Comma));
			
			return ret;
		}
		
		statement_ptr compile_simple_statement(CompilerContext& ctx, TokensIterator& it);
		
		statement_ptr compile_block_statement(CompilerContext& ctx, TokensIterator& it, possible_flow pf);
		
		statement_ptr compile_for_statement(CompilerContext& ctx, TokensIterator& it, possible_flow pf);
		
		statement_ptr compile_while_statement(CompilerContext& ctx, TokensIterator& it, possible_flow pf);
		
		statement_ptr compile_do_statement(CompilerContext& ctx, TokensIterator& it, possible_flow pf);
		
		statement_ptr compile_if_statement(CompilerContext& ctx, TokensIterator& it, possible_flow pf);
		
		statement_ptr compile_switch_statement(CompilerContext& ctx, TokensIterator& it, possible_flow pf);
		
		statement_ptr compile_var_statement(CompilerContext& ctx, TokensIterator& it);
		
		statement_ptr compile_break_statement(CompilerContext& ctx, TokensIterator& it, possible_flow pf);
		
		statement_ptr compile_continue_statement(CompilerContext& ctx, TokensIterator& it, possible_flow pf);
		
		statement_ptr compile_return_statement(CompilerContext& ctx, TokensIterator& it, possible_flow pf);
		
		statement_ptr compile_statement(CompilerContext& ctx, TokensIterator& it, possible_flow pf, bool in_switch) {
			if (it->isReservedToken()) {
				switch (it->getReservedToken()) {
					case ReservedToken::KwFor:
						return compile_for_statement(ctx, it, pf.add_loop());
					case ReservedToken::KwWhile:
						return compile_while_statement(ctx, it, pf.add_loop());
					case ReservedToken::KwDo:
						return compile_do_statement(ctx, it, pf.add_loop());
					case ReservedToken::KwIf:
						return compile_if_statement(ctx, it, pf);
					case ReservedToken::KwSwitch:
						return compile_switch_statement(ctx, it, pf.add_switch());
					case ReservedToken::KwBreak:
						return compile_break_statement(ctx, it, pf);
					case ReservedToken::KwContinue:
						return compile_continue_statement(ctx, it, pf);
					case ReservedToken::KwReturn:
						return compile_return_statement(ctx, it, pf);
					default:
						break;
				}
			}
			
			if (is_typename(ctx, it)) {
				if (in_switch) {
					throw syntaxError("Declarations in switch block are not allowed", it->getLineNumber(), it->getCharIndex());
				} else {
					return compile_var_statement(ctx, it);
				}
			}
			
			if (it->hasValue(ReservedToken::OpenCurly)) {
				return compile_block_statement(ctx, it, pf);
			}
			
			return compile_simple_statement(ctx, it);
		}
		
		statement_ptr compile_simple_statement(CompilerContext& ctx, TokensIterator& it) {
			statement_ptr ret = create_simple_statement(buildVoidExpression(ctx, it));
			parse_token_value(ctx, it, ReservedToken::Semicolon);
			return ret;
		}
		
		statement_ptr compile_for_statement(CompilerContext& ctx, TokensIterator& it, possible_flow pf) {
			auto _ = ctx.scope();
		
			parse_token_value(ctx, it, ReservedToken::KwFor);
			parse_token_value(ctx, it, ReservedToken::OpenRound);
			
			std::vector<Expression<Lvalue>::Ptr> decls;
			Expression<Void>::Ptr expr1;
			
			if (is_typename(ctx, it)) {
				decls = compile_variable_declaration(ctx, it);
			} else {
				expr1 = buildVoidExpression(ctx, it);
			}
		
			parse_token_value(ctx, it, ReservedToken::Semicolon);
			
			Expression<Number>::Ptr expr2 = buildNumberExpression(ctx, it);
			parse_token_value(ctx, it, ReservedToken::Semicolon);
			
			Expression<Void>::Ptr expr3 = buildVoidExpression(ctx, it);
			parse_token_value(ctx, it, ReservedToken::CloseRound);
			
			statement_ptr block = compile_block_statement(ctx, it, pf);
			
			if (!decls.empty()) {
				return create_for_statement(std::move(decls), std::move(expr2), std::move(expr3), std::move(block));
			} else {
				return create_for_statement(std::move(expr1), std::move(expr2), std::move(expr3), std::move(block));
			}
		}
		
		statement_ptr compile_while_statement(CompilerContext& ctx, TokensIterator& it, possible_flow pf) {
			parse_token_value(ctx, it, ReservedToken::KwWhile);

			parse_token_value(ctx, it, ReservedToken::OpenRound);
			Expression<Number>::Ptr expr = buildNumberExpression(ctx, it);
			parse_token_value(ctx, it, ReservedToken::CloseRound);
			
			statement_ptr block = compile_block_statement(ctx, it, pf);
			
			return create_while_statement(std::move(expr), std::move(block));
		}
		
		statement_ptr compile_do_statement(CompilerContext& ctx, TokensIterator& it, possible_flow pf) {
			parse_token_value(ctx, it, ReservedToken::KwDo);
			
			statement_ptr block = compile_block_statement(ctx, it, pf);
			
			parse_token_value(ctx, it, ReservedToken::KwWhile);
			
			parse_token_value(ctx, it, ReservedToken::OpenRound);
			Expression<Number>::Ptr expr = buildNumberExpression(ctx, it);
			parse_token_value(ctx, it, ReservedToken::CloseRound);
			
			return create_do_statement(std::move(expr), std::move(block));
		}
		
		statement_ptr compile_if_statement(CompilerContext& ctx, TokensIterator& it, possible_flow pf) {
			auto _ = ctx.scope();
			parse_token_value(ctx, it, ReservedToken::KwIf);
			
			parse_token_value(ctx, it, ReservedToken::OpenRound);
			
			std::vector<Expression<Lvalue>::Ptr> decls;
			
			if (is_typename(ctx, it)) {
				decls = compile_variable_declaration(ctx, it);
				parse_token_value(ctx, it, ReservedToken::Semicolon);
			}
			
			std::vector<Expression<Number>::Ptr> exprs;
			std::vector<statement_ptr> stmts;
			
			exprs.emplace_back(buildNumberExpression(ctx, it));
			parse_token_value(ctx, it, ReservedToken::CloseRound);
			stmts.emplace_back(compile_block_statement(ctx, it, pf));
			
			while (it->hasValue(ReservedToken::KwElif)) {
				++it;
				parse_token_value(ctx, it, ReservedToken::OpenRound);
				exprs.emplace_back(buildNumberExpression(ctx, it));
				parse_token_value(ctx, it, ReservedToken::CloseRound);
				stmts.emplace_back(compile_block_statement(ctx, it, pf));
			}
			
			if (it->hasValue(ReservedToken::KwElse)) {
				++it;
				stmts.emplace_back(compile_block_statement(ctx, it, pf));
			} else {
				stmts.emplace_back(create_block_statement({}));
			}
			
			return create_if_statement(std::move(decls), std::move(exprs), std::move(stmts));
		}
		
		 statement_ptr compile_switch_statement(CompilerContext& ctx, TokensIterator& it, possible_flow pf) {
		 	auto _ = ctx.scope();
			parse_token_value(ctx, it, ReservedToken::KwSwitch);
			
			parse_token_value(ctx, it, ReservedToken::OpenRound);
			
			std::vector<Expression<Lvalue>::Ptr> decls;
			
			if (is_typename(ctx, it)) {
				decls = compile_variable_declaration(ctx, it);
				parse_token_value(ctx, it, ReservedToken::Semicolon);
			}
			
			Expression<Number>::Ptr expr = buildNumberExpression(ctx, it);
			parse_token_value(ctx, it, ReservedToken::CloseRound);
			
			std::vector<statement_ptr> stmts;
			std::unordered_map<Number, size_t> cases;
			size_t dflt = size_t(-1);
			
			parse_token_value(ctx, it, ReservedToken::OpenCurly);
			
			while (!it->hasValue(ReservedToken::CloseCurly)) {
				if (it->hasValue(ReservedToken::KwCase)) {
					++it;
					if (!it->isNumber()) {
						throw unexpected_syntax(it);
					}
					cases.emplace(it->getNumber(), stmts.size());
					++it;
					parse_token_value(ctx, it, ReservedToken::Colon);
				} else if (it->hasValue(ReservedToken::KwDefault)) {
					++it;
					dflt = stmts.size();
					parse_token_value(ctx, it, ReservedToken::Colon);
				} else {
					stmts.emplace_back(compile_statement(ctx, it, pf, true));
				}
			}
			
			++it;
			
			if (dflt == size_t(-1)) {
				dflt = stmts.size();
			}
			
			return create_switch_statement(std::move(decls), std::move(expr), std::move(stmts), std::move(cases), dflt);
		}
	
		statement_ptr compile_var_statement(CompilerContext& ctx, TokensIterator& it) {
			std::vector<Expression<Lvalue>::Ptr> decls = compile_variable_declaration(ctx, it);
			parse_token_value(ctx, it, ReservedToken::Semicolon);
			return create_local_declaration_statement(std::move(decls));
		}
		
		statement_ptr compile_break_statement(CompilerContext& ctx, TokensIterator& it, possible_flow pf) {
			if (pf.break_level == 0) {
				throw unexpected_syntax(it);
			}
			
			parse_token_value(ctx, it, ReservedToken::KwBreak);
			
			double break_level;
			
			if (it->isNumber()) {
				break_level = it->getNumber();
			
				if (break_level < 1 || break_level != int(break_level) || break_level > pf.break_level) {
					throw syntaxError("Invalid break value", it->getLineNumber(), it->getCharIndex());
				}
				
				++it;
			} else {
				break_level = 1;
			}
			
			
			parse_token_value(ctx, it, ReservedToken::Semicolon);
			
			return create_break_statement(int(break_level));
		}
		
		statement_ptr compile_continue_statement(CompilerContext& ctx, TokensIterator& it, possible_flow pf){
			if (!pf.can_continue) {
				throw unexpected_syntax(it);
			}
			parse_token_value(ctx, it, ReservedToken::KwContinue);
			parse_token_value(ctx, it, ReservedToken::Semicolon);
			return create_continue_statement();
		}
		
		statement_ptr compile_return_statement(CompilerContext& ctx, TokensIterator& it, possible_flow pf){
			parse_token_value(ctx, it, ReservedToken::KwReturn);
			
			if (pf.return_type_id == TypeRegistry::getVoidHandle()) {
				parse_token_value(ctx, it, ReservedToken::Semicolon);
				return create_return_void_statement();
			} else {
				Expression<Lvalue>::Ptr expr = buildInitializationExpression(ctx, it, pf.return_type_id, true);
				parse_token_value(ctx, it, ReservedToken::Semicolon);
				return create_return_statement(std::move(expr));
			}
		}
		
		
		std::vector<statement_ptr> compile_block_contents(CompilerContext& ctx, TokensIterator& it, possible_flow pf) {
			std::vector<statement_ptr> ret;
			
			if (it->hasValue(ReservedToken::OpenCurly)) {
				parse_token_value(ctx, it, ReservedToken::OpenCurly);
				
				while (!it->hasValue(ReservedToken::CloseCurly)) {
					ret.push_back(compile_statement(ctx, it, pf, false));
				}
				
				parse_token_value(ctx, it, ReservedToken::CloseCurly);
			} else {
				ret.push_back(compile_statement(ctx, it, pf, false));
			}
			
			return ret;
		}
		
		statement_ptr compile_block_statement(CompilerContext& ctx, TokensIterator& it, possible_flow pf) {
			auto _ = ctx.scope();
			std::vector<statement_ptr> block = compile_block_contents(ctx, it, pf);
			return create_block_statement(std::move(block));
		}
	}

	void parse_token_value(CompilerContext&, TokensIterator& it, const TokenValue& value) {
		if (it->hasValue(value)) {
			++it;
			return;
		}
		throw expectedSyntaxError(std::to_string(value), it->getLineNumber(), it->getCharIndex());
	}
	
	std::string parse_declaration_name(CompilerContext& ctx, TokensIterator& it) {
		if (!it->isIdentifier()) {
			throw unexpected_syntax(it);
		}

		std::string ret = it->getIdentifier().name;
		
		if (!ctx.canDeclare(ret)) {
			throw alreadyDeclaredError(ret, it->getLineNumber(), it->getCharIndex());
		}

		++it;
		
		return ret;
	}

	TypeHandle parse_type(CompilerContext& ctx, TokensIterator& it) {
		if (!it->isReservedToken()) {
			throw unexpected_syntax(it);
		}
		
		TypeHandle t = nullptr;
		
		switch (it->getReservedToken()) {
			case ReservedToken::KwVoid:
				t = ctx.getHandle(SimpleType::Void);
				++it;
				break;
			case ReservedToken::KwNumber:
				t = ctx.getHandle(SimpleType::Number);
				++it;
				break;
			case ReservedToken::KwString:
				t = ctx.getHandle(SimpleType::String);
				++it;
				break;
			case ReservedToken::OpenSquare:
				{
					TupleType tt;
					++it;
					while (!it->hasValue(ReservedToken::CloseSquare)) {
						if (!tt.innerTypeId.empty()) {
							parse_token_value(ctx, it, ReservedToken::Comma);
						}
						tt.innerTypeId.push_back(parse_type(ctx, it));
					}
					++it;
					t = ctx.getHandle(std::move(tt));
				}
				break;
			default:
				throw unexpected_syntax(it);
		}
		
		while (it->isReservedToken()) {
			switch (it->getReservedToken()) {
				case ReservedToken::OpenSquare:
					parse_token_value(ctx, ++it, ReservedToken::CloseSquare);
					t = ctx.getHandle(ArrayType{t});
					break;
				case ReservedToken::OpenRound:
					{
						FunctionType ft;
						ft.returnTypeId = t;
						++it;
						while (!it->hasValue(ReservedToken::CloseRound)) {
							if (!ft.paramTypeId.empty()) {
								parse_token_value(ctx, it, ReservedToken::Comma);
							}
							TypeHandle param_type = parse_type(ctx, it);
							if (it->hasValue(ReservedToken::BitwiseAnd)) {
								ft.paramTypeId.push_back({param_type, true});
								++it;
							} else {
								ft.paramTypeId.push_back({param_type, false});
							}
						}
						++it;
						t = ctx.getHandle(ft);
					}
					break;
				default:
					return t;
			}
		}
		
		return t;
	}
	
	shared_statement_ptr compile_function_block(CompilerContext& ctx, TokensIterator& it, TypeHandle return_type_id) {
		std::vector<statement_ptr> block = compile_block_contents(ctx, it, possible_flow::in_function(return_type_id));
		if (return_type_id != TypeRegistry::getVoidHandle()) {
			block.emplace_back(create_return_statement(buildDefaultInitialization(return_type_id)));
		}
		return create_shared_block_statement(std::move(block));
	}
	
	RuntimeContext compile(
		TokensIterator& it,
		const std::vector<std::pair<std::string, function> >& external_functions,
		std::vector<std::string> public_declarations
	) {
		CompilerContext ctx;
		
		for (const std::pair<std::string, function>& p : external_functions) {
			GetCharacter get = [i = 0, &p]() mutable {
				if (i < p.first.size()){
					return int(p.first[i++]);
				} else {
					return -1;
				}
			};
			
			PushBackStream stream(&get);
			
			TokensIterator function_it(stream);
		
			function_declaration decl = parse_function_declaration(ctx, function_it);
			
			ctx.createFunction(decl.name, decl.type_id);
		}
		
		std::unordered_map<std::string, TypeHandle> public_function_types;
		
		for (const std::string& f : public_declarations) {
			GetCharacter get = [i = 0, &f]() mutable {
				if (i < f.size()){
					return int(f[i++]);
				} else {
					return -1;
				}
			};
			
			PushBackStream stream(&get);
			
			TokensIterator function_it(stream);
		
			function_declaration decl = parse_function_declaration(ctx, function_it);
			
			public_function_types.emplace(decl.name, decl.type_id);
		}

		std::vector<Expression<Lvalue>::Ptr> initializers;
		
		std::vector<incomplete_function> incomplete_functions;
		std::unordered_map<std::string, size_t> public_functions;
		
		while (it) {
			if (!std::holds_alternative<ReservedToken>(it->getValue())) {
				throw unexpected_syntax(it);
			}
		
			bool public_function = false;
			
			switch (it->getReservedToken()) {
				case ReservedToken::KwPublic:
					public_function = true;
					if (!(++it)->hasValue(ReservedToken::KwFunction)) {
						throw unexpected_syntax(it);
					}
				case ReservedToken::KwFunction:
					{
						size_t line_number = it->getLineNumber();
						size_t char_index = it->getCharIndex();
						const incomplete_function& f = incomplete_functions.emplace_back(ctx, it);
						
						if (public_function) {
							auto it = public_function_types.find(f.get_decl().name);
						
							if (it != public_function_types.end() && it->second != f.get_decl().type_id) {
								throw semanticError(
									"Public function doesn't match it's declaration " + std::to_string(it->second),
									line_number,
									char_index
								);
							} else {
								public_function_types.erase(it);
							}
						
							public_functions.emplace(
								f.get_decl().name,
								external_functions.size() + incomplete_functions.size() - 1
							);
						}
						break;
					}
				default:
					for (Expression<Lvalue>::Ptr& expr : compile_variable_declaration(ctx, it)) {
						initializers.push_back(std::move(expr));
					}
					parse_token_value(ctx, it, ReservedToken::Semicolon);
					break;
			}
		}
		
		if (!public_function_types.empty()) {
			throw semanticError(
				"Public function '" + public_function_types.begin()->first + "' is not defined.",
				it->getLineNumber(),
				it->getCharIndex()
			);
		}
		
		std::vector<function> functions;
		
		functions.reserve(external_functions.size() + incomplete_functions.size());
		
		for (const std::pair<std::string, function>& p : external_functions) {
			functions.emplace_back(p.second);
		}
		
		for (incomplete_function& f : incomplete_functions) {
			functions.emplace_back(f.compile(ctx));
		}
		
		return RuntimeContext(std::move(initializers), std::move(functions), std::move(public_functions));
	}
}
