#include "IncompleteClass.hpp"
#include "Compiler.hpp"
#include "CompilerContext.hpp"
#include "Errors.hpp"
#include "Tokenizer.hpp"

namespace sharpsenLang
{
	namespace
	{
		std::string getDeclarationName(CompilerContext &ctx, TokensIterator &it)
		{
			if (!it->isIdentifier())
			{
				throw "todo error";
			}

			std::string ret = it->getIdentifier().name;

			++it;

			return ret;
		}

		std::pair<TypeHandle, std::string> GetVariableDefinition(CompilerContext &ctx, TokensIterator &it)
		{
			TypeHandle typeId = parseType(ctx, it);

			if (typeId == TypeRegistry::getVoidHandle())
			{
				throw syntaxError("Cannot declare void variable", it->getLineNumber(), it->getCharIndex());
			}

			std::vector<Expression<Lvalue>::Ptr> ret;

			if (!ret.empty())
			{
				++it;
			}

			std::string name = getDeclarationName(ctx, it);
			return {typeId, name};
		}

	}

	ClassDeclaration IncompleteClass::parseClassDeclaration(CompilerContext &ctx, TokensIterator &it)
	{
		ClassDeclaration ret;

		parseTokenValue(ctx, it, ReservedToken::KwClass);

		ClassType ct;
		ret.name = parseDeclarationName(ctx, it);

		{
			parseTokenValue(ctx, it, ReservedToken::OpenCurly);

			while (!it->hasValue(ReservedToken::CloseCurly))
			{
				if (!std::holds_alternative<ReservedToken>(it->getValue()))
				{
					throw "todo error";
				}

				bool publicFunction = false;

				switch (it->getReservedToken())
				{
				case ReservedToken::KwPublic:
					publicFunction = true;
					if (!(++it)->hasValue(ReservedToken::KwFunction))
					{
						throw "todo error";
					}
				case ReservedToken::KwFunction:
				{
					size_t lineNumber = it->getLineNumber();
					size_t charIndex = it->getCharIndex();
					const IncompleteFunction &f = _incompleteMethods.emplace_back(ctx, it);
					ret.methods.push_back(f.getDecl());

					break;
				}
				default:
					auto property = GetVariableDefinition(ctx, it);
					ret.properties.emplace_back(property.second, property.first);
					parseTokenValue(ctx, it, ReservedToken::Semicolon);
					break;
				}
			}
			++it;
		}

		ret.typeId = ctx.getHandle(ct);

		return ret;
	}

	IncompleteClass::IncompleteClass(CompilerContext &ctx, TokensIterator &it)
	{
		_decl = parseClassDeclaration(ctx, it);
		//ctx.createClass(_decl.name, _decl.typeId);
	}

	IncompleteClass::IncompleteClass(IncompleteClass &&orig) noexcept : _tokens(std::move(orig._tokens)),
																		_decl(std::move(orig._decl))
	{
	}

	const ClassDeclaration &IncompleteClass::getDecl() const
	{
		return _decl;
	}

	Function IncompleteClass::compile(CompilerContext &ctx)
	{
		for(auto& incompleted: _incompleteMethods) {
			incompleted.compile(ctx);
		}
		return nullptr;
	}
}
