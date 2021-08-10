#pragma once
#include <memory>
#include <string>
#include <unordered_map>

#include "Types.hpp"

namespace sharpsenLang
{

	enum struct IdentifierScope
	{
		GlobalVariable,
		LocalVariable,
		Function,
		Class,
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

	class ClassInfo : public IdentifierInfo
	{
	private:
		std::unordered_map<std::string, size_t> _propertiesMap;

	public:
		ClassInfo(TypeHandle typeId, size_t index, IdentifierScope scope, std::vector<std::string> properties);

		const size_t *getPropertyIndex(const std::string &name) const;
	};

	class IdentifierLookup
	{
	private:
		std::unordered_map<std::string, IdentifierInfo> _identifiers;

	protected:
		const IdentifierInfo *insertIdentifier(std::string name, TypeHandle typeId,
											   size_t index, IdentifierScope scope);
		size_t identifiersSize() const;

	public:
		virtual const IdentifierInfo *find(const std::string &name) const;

		virtual const IdentifierInfo *createIdentifier(std::string name,
													   TypeHandle typeId) = 0;

		bool canDeclare(const std::string &name) const;

		virtual ~IdentifierLookup();
	};

	class GlobalVariableLookup : public IdentifierLookup
	{
	public:
		const IdentifierInfo *createIdentifier(std::string name,
											   TypeHandle typeId) override;
	};

	class LocalVariableLookup : public IdentifierLookup
	{
	private:
		std::unique_ptr<LocalVariableLookup> _parent;
		int _nextIdentifierIndex;

	public:
		LocalVariableLookup(std::unique_ptr<LocalVariableLookup> parentLookup);

		const IdentifierInfo *find(const std::string &name) const override;

		const IdentifierInfo *createIdentifier(std::string name,
											   TypeHandle typeId) override;

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
		const IdentifierInfo *createIdentifier(std::string name,
											   TypeHandle typeId) override;
	};

	class ClassLookup
	{
	private:
		std::unordered_map<std::string, ClassInfo> _identifiers;

		size_t identifiersSize() const;

	public:
		const ClassInfo *createClass(std::string name,
									 TypeHandle typeId, std::vector<std::string> properties);

		const ClassInfo *find(const std::string &name) const;

		bool canDeclare(const std::string &name) const;
	};

	class CompilerContext
	{
	private:
		FunctionLookup _functions;
		ClassLookup _classes;
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

		const IdentifierInfo *findClass(const std::string &name) const;

		const Property *getClassProperty(const ClassType *ct, std::string_view propertyName) const;

		const IdentifierInfo *createIdentifier(std::string name, TypeHandle typeId);

		const IdentifierInfo *createParam(std::string name, TypeHandle typeId);

		const IdentifierInfo *createFunction(std::string name, TypeHandle typeId);

		const IdentifierInfo *createClass(std::string name, TypeHandle typeId, std::vector<std::string> properties);

		bool canDeclare(const std::string &name) const;

		ScopeRaii scope();
		FunctionRaii function();
	};
}
