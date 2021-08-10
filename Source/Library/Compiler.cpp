#include "Compiler.hpp"
#include "Errors.hpp"
#include "CompilerContext.hpp"
#include "Expression.hpp"
#include "IncompleteFunction.hpp"
#include "Tokenizer.hpp"
#include "RuntimeContext.hpp"
#include "Helpers.hpp"
#include "PushBackStream.hpp"
#include "IncompleteClass.hpp"

namespace sharpsenLang
{
	namespace
	{
		struct PossibleFlow
		{
			size_t breakLevel;
			bool canContinue;
			TypeHandle returnTypeId;

			PossibleFlow addSwitch()
			{
				return PossibleFlow{breakLevel + 1, canContinue, returnTypeId};
			}

			PossibleFlow addLoop()
			{
				return PossibleFlow{breakLevel + 1, true, returnTypeId};
			}

			static PossibleFlow inFunction(TypeHandle returnTypeId)
			{
				return PossibleFlow{0, false, returnTypeId};
			}
		};

		bool isTypename(const CompilerContext &ctx, const TokensIterator &it)
		{
			return std::visit(
				overloaded{
					[](ReservedToken t)
					{
						switch (t)
						{
						case ReservedToken::KwNumber:
						case ReservedToken::KwString:
						case ReservedToken::KwVoid:
						case ReservedToken::OpenSquare:
							return true;
						default:
							return false;
						}
					},
					[&ctx](const Identifier &t) -> bool
					{
						return ctx.findClass(t.name);
					},
					[](const TokenValue &)
					{
						return false;
					}},
				it->getValue());
		}

		Error unexpectedSyntax(const TokensIterator &it)
		{
			return unexpectedSyntaxError(std::to_string(it->getValue()), it->getLineNumber(), it->getCharIndex());
		}

		std::vector<Expression<Lvalue>::Ptr> compileVariableDeclaration(CompilerContext &ctx, TokensIterator &it)
		{
			TypeHandle typeId = parseType(ctx, it);

			if (typeId == TypeRegistry::getVoidHandle())
			{
				throw syntaxError("Cannot declare void variable", it->getLineNumber(), it->getCharIndex());
			}

			std::vector<Expression<Lvalue>::Ptr> ret;

			do
			{
				if (!ret.empty())
				{
					++it;
				}

				std::string name = parseDeclarationName(ctx, it);

				if (it->hasValue(ReservedToken::OpenRound))
				{
					++it;
					ret.emplace_back(buildInitializationExpression(ctx, it, typeId, false));
					parseTokenValue(ctx, it, ReservedToken::CloseRound);
				}
				else if (it->hasValue(ReservedToken::Assign))
				{
					++it;
					ret.emplace_back(buildInitializationExpression(ctx, it, typeId, false));
				}
				else
				{
					ret.emplace_back(buildDefaultInitialization(typeId));
				}

				ctx.createIdentifier(std::move(name), typeId);
			} while (it->hasValue(ReservedToken::Comma));

			return ret;
		}

		StatementPtr compileSimpleStatement(CompilerContext &ctx, TokensIterator &it);

		StatementPtr compileBlockStatement(CompilerContext &ctx, TokensIterator &it, PossibleFlow pf);

		StatementPtr compileForStatement(CompilerContext &ctx, TokensIterator &it, PossibleFlow pf);

		StatementPtr compileWhileStatement(CompilerContext &ctx, TokensIterator &it, PossibleFlow pf);

		StatementPtr compileDoStatement(CompilerContext &ctx, TokensIterator &it, PossibleFlow pf);

		StatementPtr compileIfStatement(CompilerContext &ctx, TokensIterator &it, PossibleFlow pf);

		StatementPtr compileSwitchStatement(CompilerContext &ctx, TokensIterator &it, PossibleFlow pf);

		StatementPtr compileVarStatement(CompilerContext &ctx, TokensIterator &it);

		StatementPtr compileBreakStatement(CompilerContext &ctx, TokensIterator &it, PossibleFlow pf);

		StatementPtr compileContinueStatement(CompilerContext &ctx, TokensIterator &it, PossibleFlow pf);

		StatementPtr compileReturnStatement(CompilerContext &ctx, TokensIterator &it, PossibleFlow pf);

		StatementPtr compileStatement(CompilerContext &ctx, TokensIterator &it, PossibleFlow pf, bool inSwitch)
		{
			if (it->isReservedToken())
			{
				switch (it->getReservedToken())
				{
				case ReservedToken::KwFor:
					return compileForStatement(ctx, it, pf.addLoop());
				case ReservedToken::KwWhile:
					return compileWhileStatement(ctx, it, pf.addLoop());
				case ReservedToken::KwDo:
					return compileDoStatement(ctx, it, pf.addLoop());
				case ReservedToken::KwIf:
					return compileIfStatement(ctx, it, pf);
				case ReservedToken::KwSwitch:
					return compileSwitchStatement(ctx, it, pf.addSwitch());
				case ReservedToken::KwBreak:
					return compileBreakStatement(ctx, it, pf);
				case ReservedToken::KwContinue:
					return compileContinueStatement(ctx, it, pf);
				case ReservedToken::KwReturn:
					return compileReturnStatement(ctx, it, pf);
				default:
					break;
				}
			}

			if (isTypename(ctx, it))
			{
				if (inSwitch)
				{
					throw syntaxError("Declarations in switch block are not allowed", it->getLineNumber(), it->getCharIndex());
				}
				else
				{
					return compileVarStatement(ctx, it);
				}
			}

			if (it->hasValue(ReservedToken::OpenCurly))
			{
				return compileBlockStatement(ctx, it, pf);
			}

			return compileSimpleStatement(ctx, it);
		}

		StatementPtr compileSimpleStatement(CompilerContext &ctx, TokensIterator &it)
		{
			StatementPtr ret = createSimpleStatement(buildVoidExpression(ctx, it));
			parseTokenValue(ctx, it, ReservedToken::Semicolon);
			return ret;
		}

		StatementPtr compileForStatement(CompilerContext &ctx, TokensIterator &it, PossibleFlow pf)
		{
			auto _ = ctx.scope();

			parseTokenValue(ctx, it, ReservedToken::KwFor);
			parseTokenValue(ctx, it, ReservedToken::OpenRound);

			std::vector<Expression<Lvalue>::Ptr> decls;
			Expression<Void>::Ptr expr1;

			if (isTypename(ctx, it))
			{
				decls = compileVariableDeclaration(ctx, it);
			}
			else
			{
				expr1 = buildVoidExpression(ctx, it);
			}

			parseTokenValue(ctx, it, ReservedToken::Semicolon);

			Expression<Number>::Ptr expr2 = buildNumberExpression(ctx, it);
			parseTokenValue(ctx, it, ReservedToken::Semicolon);

			Expression<Void>::Ptr expr3 = buildVoidExpression(ctx, it);
			parseTokenValue(ctx, it, ReservedToken::CloseRound);

			StatementPtr block = compileBlockStatement(ctx, it, pf);

			if (!decls.empty())
			{
				return createForStatement(std::move(decls), std::move(expr2), std::move(expr3), std::move(block));
			}
			else
			{
				return createForStatement(std::move(expr1), std::move(expr2), std::move(expr3), std::move(block));
			}
		}

		StatementPtr compileWhileStatement(CompilerContext &ctx, TokensIterator &it, PossibleFlow pf)
		{
			parseTokenValue(ctx, it, ReservedToken::KwWhile);

			parseTokenValue(ctx, it, ReservedToken::OpenRound);
			Expression<Number>::Ptr expr = buildNumberExpression(ctx, it);
			parseTokenValue(ctx, it, ReservedToken::CloseRound);

			StatementPtr block = compileBlockStatement(ctx, it, pf);

			return createWhileStatement(std::move(expr), std::move(block));
		}

		StatementPtr compileDoStatement(CompilerContext &ctx, TokensIterator &it, PossibleFlow pf)
		{
			parseTokenValue(ctx, it, ReservedToken::KwDo);

			StatementPtr block = compileBlockStatement(ctx, it, pf);

			parseTokenValue(ctx, it, ReservedToken::KwWhile);

			parseTokenValue(ctx, it, ReservedToken::OpenRound);
			Expression<Number>::Ptr expr = buildNumberExpression(ctx, it);
			parseTokenValue(ctx, it, ReservedToken::CloseRound);

			return createDoStatement(std::move(expr), std::move(block));
		}

		StatementPtr compileIfStatement(CompilerContext &ctx, TokensIterator &it, PossibleFlow pf)
		{
			auto _ = ctx.scope();
			parseTokenValue(ctx, it, ReservedToken::KwIf);

			parseTokenValue(ctx, it, ReservedToken::OpenRound);

			std::vector<Expression<Lvalue>::Ptr> decls;

			if (isTypename(ctx, it))
			{
				decls = compileVariableDeclaration(ctx, it);
				parseTokenValue(ctx, it, ReservedToken::Semicolon);
			}

			std::vector<Expression<Number>::Ptr> exprs;
			std::vector<StatementPtr> stmts;

			exprs.emplace_back(buildNumberExpression(ctx, it));
			parseTokenValue(ctx, it, ReservedToken::CloseRound);
			stmts.emplace_back(compileBlockStatement(ctx, it, pf));

			while (it->hasValue(ReservedToken::KwElif))
			{
				++it;
				parseTokenValue(ctx, it, ReservedToken::OpenRound);
				exprs.emplace_back(buildNumberExpression(ctx, it));
				parseTokenValue(ctx, it, ReservedToken::CloseRound);
				stmts.emplace_back(compileBlockStatement(ctx, it, pf));
			}

			if (it->hasValue(ReservedToken::KwElse))
			{
				++it;
				stmts.emplace_back(compileBlockStatement(ctx, it, pf));
			}
			else
			{
				stmts.emplace_back(createBlockStatement({}));
			}

			return createIfStatement(std::move(decls), std::move(exprs), std::move(stmts));
		}

		StatementPtr compileSwitchStatement(CompilerContext &ctx, TokensIterator &it, PossibleFlow pf)
		{
			auto _ = ctx.scope();
			parseTokenValue(ctx, it, ReservedToken::KwSwitch);

			parseTokenValue(ctx, it, ReservedToken::OpenRound);

			std::vector<Expression<Lvalue>::Ptr> decls;

			if (isTypename(ctx, it))
			{
				decls = compileVariableDeclaration(ctx, it);
				parseTokenValue(ctx, it, ReservedToken::Semicolon);
			}

			Expression<Number>::Ptr expr = buildNumberExpression(ctx, it);
			parseTokenValue(ctx, it, ReservedToken::CloseRound);

			std::vector<StatementPtr> stmts;
			std::unordered_map<Number, size_t> cases;
			size_t dflt = size_t(-1);

			parseTokenValue(ctx, it, ReservedToken::OpenCurly);

			while (!it->hasValue(ReservedToken::CloseCurly))
			{
				if (it->hasValue(ReservedToken::KwCase))
				{
					++it;
					if (!it->isNumber())
					{
						throw unexpectedSyntax(it);
					}
					cases.emplace(it->getNumber(), stmts.size());
					++it;
					parseTokenValue(ctx, it, ReservedToken::Colon);
				}
				else if (it->hasValue(ReservedToken::KwDefault))
				{
					++it;
					dflt = stmts.size();
					parseTokenValue(ctx, it, ReservedToken::Colon);
				}
				else
				{
					stmts.emplace_back(compileStatement(ctx, it, pf, true));
				}
			}

			++it;

			if (dflt == size_t(-1))
			{
				dflt = stmts.size();
			}

			return createSwitchStatement(std::move(decls), std::move(expr), std::move(stmts), std::move(cases), dflt);
		}

		StatementPtr compileVarStatement(CompilerContext &ctx, TokensIterator &it)
		{
			std::vector<Expression<Lvalue>::Ptr> decls = compileVariableDeclaration(ctx, it);
			parseTokenValue(ctx, it, ReservedToken::Semicolon);
			return createLocalDeclarationStatement(std::move(decls));
		}

		StatementPtr compileBreakStatement(CompilerContext &ctx, TokensIterator &it, PossibleFlow pf)
		{
			if (pf.breakLevel == 0)
			{
				throw unexpectedSyntax(it);
			}

			parseTokenValue(ctx, it, ReservedToken::KwBreak);

			double breakLevel;

			if (it->isNumber())
			{
				breakLevel = it->getNumber();

				if (breakLevel < 1 || breakLevel != int(breakLevel) || breakLevel > pf.breakLevel)
				{
					throw syntaxError("Invalid break value", it->getLineNumber(), it->getCharIndex());
				}

				++it;
			}
			else
			{
				breakLevel = 1;
			}

			parseTokenValue(ctx, it, ReservedToken::Semicolon);

			return createBreakStatement(int(breakLevel));
		}

		StatementPtr compileContinueStatement(CompilerContext &ctx, TokensIterator &it, PossibleFlow pf)
		{
			if (!pf.canContinue)
			{
				throw unexpectedSyntax(it);
			}
			parseTokenValue(ctx, it, ReservedToken::KwContinue);
			parseTokenValue(ctx, it, ReservedToken::Semicolon);
			return createContinueStatement();
		}

		StatementPtr compileReturnStatement(CompilerContext &ctx, TokensIterator &it, PossibleFlow pf)
		{
			parseTokenValue(ctx, it, ReservedToken::KwReturn);

			if (pf.returnTypeId == TypeRegistry::getVoidHandle())
			{
				parseTokenValue(ctx, it, ReservedToken::Semicolon);
				return createReturnVoidStatement();
			}
			else
			{
				Expression<Lvalue>::Ptr expr = buildInitializationExpression(ctx, it, pf.returnTypeId, true);
				parseTokenValue(ctx, it, ReservedToken::Semicolon);
				return createReturnStatement(std::move(expr));
			}
		}

		std::vector<StatementPtr> compileBlockContents(CompilerContext &ctx, TokensIterator &it, PossibleFlow pf)
		{
			std::vector<StatementPtr> ret;

			if (it->hasValue(ReservedToken::OpenCurly))
			{
				parseTokenValue(ctx, it, ReservedToken::OpenCurly);

				while (!it->hasValue(ReservedToken::CloseCurly))
				{
					ret.push_back(compileStatement(ctx, it, pf, false));
				}

				parseTokenValue(ctx, it, ReservedToken::CloseCurly);
			}
			else
			{
				ret.push_back(compileStatement(ctx, it, pf, false));
			}

			return ret;
		}

		StatementPtr compileBlockStatement(CompilerContext &ctx, TokensIterator &it, PossibleFlow pf)
		{
			auto _ = ctx.scope();
			std::vector<StatementPtr> block = compileBlockContents(ctx, it, pf);
			return createBlockStatement(std::move(block));
		}
	}

	void parseTokenValue(CompilerContext &, TokensIterator &it, const TokenValue &value)
	{
		if (it->hasValue(value))
		{
			++it;
			return;
		}
		throw expectedSyntaxError(std::to_string(value), it->getLineNumber(), it->getCharIndex());
	}

	std::string parseDeclarationName(CompilerContext &ctx, TokensIterator &it)
	{
		if (!it->isIdentifier())
		{
			throw unexpectedSyntax(it);
		}

		std::string ret = it->getIdentifier().name;

		if (!ctx.canDeclare(ret))
		{
			throw alreadyDeclaredError(ret, it->getLineNumber(), it->getCharIndex());
		}

		++it;

		return ret;
	}

	TypeHandle parseType(CompilerContext &ctx, TokensIterator &it)
	{
		TypeHandle t = nullptr;

		if (it->isReservedToken())
		{
			switch (it->getReservedToken())
			{
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
				while (!it->hasValue(ReservedToken::CloseSquare))
				{
					if (!tt.innerTypeId.empty())
					{
						parseTokenValue(ctx, it, ReservedToken::Comma);
					}
					tt.innerTypeId.push_back(parseType(ctx, it));
				}
				++it;
				t = ctx.getHandle(std::move(tt));
			}
			break;
			default:
				throw unexpectedSyntax(it);
			}
		}
		else if (it->isIdentifier())
		{
			if (auto classType = ctx.findClass(it->getIdentifier().name))
			{
				t = classType->typeId();
				++it;
			}
			else
			{
				throw unexpectedSyntax(it);
			}
		}
		else
		{
			throw unexpectedSyntax(it);
		}

		while (it->isReservedToken())
		{
			switch (it->getReservedToken())
			{
			case ReservedToken::OpenSquare:
				parseTokenValue(ctx, ++it, ReservedToken::CloseSquare);
				t = ctx.getHandle(ArrayType{t});
				break;
			case ReservedToken::OpenRound:
			{
				FunctionType ft;
				ft.returnTypeId = t;
				++it;
				while (!it->hasValue(ReservedToken::CloseRound))
				{
					if (!ft.paramTypeId.empty())
					{
						parseTokenValue(ctx, it, ReservedToken::Comma);
					}
					TypeHandle param_type = parseType(ctx, it);
					if (it->hasValue(ReservedToken::BitwiseAnd))
					{
						ft.paramTypeId.push_back({param_type, true});
						++it;
					}
					else
					{
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

	SharedStatementPtr compileFunctionBlock(CompilerContext &ctx, TokensIterator &it, TypeHandle returnTypeId)
	{
		std::vector<StatementPtr> block = compileBlockContents(ctx, it, PossibleFlow::inFunction(returnTypeId));
		if (returnTypeId != TypeRegistry::getVoidHandle())
		{
			block.emplace_back(createReturnStatement(buildDefaultInitialization(returnTypeId)));
		}
		return createSharedBlockStatement(std::move(block));
	}

	RuntimeContext compile(
		TokensIterator &it,
		const std::vector<std::pair<std::string, Function>> &externalFunctions,
		std::vector<std::string> public_declarations)
	{
		CompilerContext ctx;

		for (const std::pair<std::string, Function> &p : externalFunctions)
		{
			GetCharacter get = [i = 0, &p]() mutable
			{
				if (i < p.first.size())
				{
					return int(p.first[i++]);
				}
				else
				{
					return -1;
				}
			};

			PushBackStream stream(&get);

			TokensIterator function_it(stream);

			FunctionDeclaration decl = parseFunctionDeclaration(ctx, function_it);

			ctx.createFunction(decl.name, decl.typeId);
		}

		std::unordered_map<std::string, TypeHandle> public_function_types;

		for (const std::string &f : public_declarations)
		{
			GetCharacter get = [i = 0, &f]() mutable
			{
				if (i < f.size())
				{
					return int(f[i++]);
				}
				else
				{
					return -1;
				}
			};

			PushBackStream stream(&get);

			TokensIterator function_it(stream);

			FunctionDeclaration decl = parseFunctionDeclaration(ctx, function_it);

			public_function_types.emplace(decl.name, decl.typeId);
		}

		std::vector<Expression<Lvalue>::Ptr> initializers;

		std::vector<IncompleteFunction> incompleteFunctions;
		std::vector<IncompleteClass> incompleteClasses;
		std::unordered_map<std::string, size_t> publicFunctions;

		while (it)
		{
			if (!std::holds_alternative<ReservedToken>(it->getValue()))
			{
				throw unexpectedSyntax(it);
			}

			bool publicFunction = false;

			switch (it->getReservedToken())
			{
			case ReservedToken::KwPublic:
				publicFunction = true;
				if (!(++it)->hasValue(ReservedToken::KwFunction))
				{
					throw unexpectedSyntax(it);
				}
			case ReservedToken::KwFunction:
			{
				size_t lineNumber = it->getLineNumber();
				size_t charIndex = it->getCharIndex();
				const IncompleteFunction &f = incompleteFunctions.emplace_back(ctx, it);

				if (publicFunction)
				{
					auto it = public_function_types.find(f.getDecl().name);

					if (it != public_function_types.end() && it->second != f.getDecl().typeId)
					{
						throw semanticError(
							"Public function doesn't match it's declaration " + std::to_string(it->second),
							lineNumber,
							charIndex);
					}
					else
					{
						public_function_types.erase(it);
					}

					publicFunctions.emplace(
						f.getDecl().name,
						externalFunctions.size() + incompleteFunctions.size() - 1);
				}
				break;
			}
			case ReservedToken::KwClass:
			{
				size_t lineNumber = it->getLineNumber();
				size_t charIndex = it->getCharIndex();
				incompleteClasses.emplace_back(ctx, it);

				break;
			}
			default:
				for (Expression<Lvalue>::Ptr &expr : compileVariableDeclaration(ctx, it))
				{
					initializers.push_back(std::move(expr));
				}
				parseTokenValue(ctx, it, ReservedToken::Semicolon);
				break;
			}
		}

		if (!public_function_types.empty())
		{
			throw semanticError(
				"Public function '" + public_function_types.begin()->first + "' is not defined.",
				it->getLineNumber(),
				it->getCharIndex());
		}

		std::vector<Function> functions;
		std::vector<Class> classes;

		functions.reserve(externalFunctions.size() + incompleteFunctions.size());

		for (const std::pair<std::string, Function> &p : externalFunctions)
		{
			functions.emplace_back(p.second);
		}

		for (IncompleteFunction &f : incompleteFunctions)
		{
			functions.emplace_back(f.compile(ctx));
		}

		for (IncompleteClass &c : incompleteClasses)
		{
			classes.emplace_back(c.compile(ctx));
		}

		return RuntimeContext(std::move(initializers), std::move(functions), std::move(classes), std::move(publicFunctions));
	}
}
