#pragma once

#include <unordered_map>
#include <memory>
#include <string>

#include "Types.hpp"

namespace sharpsenLang
{

	enum struct IdentifierScope
	{
		global_variable,
		local_variable,
		function,
	};

	class IdentifierInfo
	{
	private:
		TypeHandle _typeId;
		size_t _index;
		IdentifierScope _scope;

	public:
		IdentifierInfo(TypeHandle typeId, size_t index, IdentifierScope scope);

		TypeHandle typeId() const;

		size_t index() const;

		IdentifierScope getScope() const;
	};

	class IdentifierLookup
	{
	private:
		std::unordered_map<std::string, IdentifierInfo> _identifiers;

	protected:
		const IdentifierInfo *insertIdentifier(std::string name, TypeHandle typeId, size_t index, IdentifierScope scope);
		size_t identifiersSize() const;

	public:
		virtual const IdentifierInfo *find(const std::string &name) const;

		virtual const IdentifierInfo *createIdentifier(std::string name, TypeHandle typeId) = 0;

		bool canDeclare(const std::string &name) const;

		virtual ~IdentifierLookup();
	};

	class GlobalVariableLookup : public IdentifierLookup
	{
	public:
		const IdentifierInfo *createIdentifier(std::string name, TypeHandle typeId) override;
	};

	class LocalVariableLookup : public IdentifierLookup
	{
	private:
		std::unique_ptr<LocalVariableLookup> _parent;
		int _nextIdentifierIndex;

	public:
		LocalVariableLookup(std::unique_ptr<LocalVariableLookup> parentLookup);

		const IdentifierInfo *find(const std::string &name) const override;

		const IdentifierInfo *createIdentifier(std::string name, TypeHandle typeId) override;

		std::unique_ptr<LocalVariableLookup> detachParent();
	};

	class ParamLookup : public LocalVariableLookup
	{
	private:
		int _nextParamIndex;

	public:
		ParamLookup();

		const IdentifierInfo *createParam(std::string name, TypeHandle typeId);
	};

	class FunctionLookup : public IdentifierLookup
	{
	public:
		const IdentifierInfo *createIdentifier(std::string name, TypeHandle typeId) override;
	};

	class CompilerContext
	{
	private:
		FunctionLookup _functions;
		GlobalVariableLookup _globals;
		ParamLookup *_params;
		std::unique_ptr<LocalVariableLookup> _locals;
		TypeRegistry _types;

		class ScopeRaii
		{
		private:
			CompilerContext &_context;

		public:
			ScopeRaii(CompilerContext &context);
			~ScopeRaii();
		};

		class FunctionRaii
		{
		private:
			CompilerContext &_context;

		public:
			FunctionRaii(CompilerContext &context);
			~FunctionRaii();
		};

		void enterFunction();
		void enterScope();
		void leaveScope();

	public:
		CompilerContext();

		TypeHandle getHandle(const Type &t);

		const IdentifierInfo *find(const std::string &name) const;

		const IdentifierInfo *createIdentifier(std::string name, TypeHandle typeId);

		const IdentifierInfo *createParam(std::string name, TypeHandle typeId);

		const IdentifierInfo *createFunction(std::string name, TypeHandle typeId);

		bool canDeclare(const std::string &name) const;

		ScopeRaii scope();
		FunctionRaii function();
	};
}
