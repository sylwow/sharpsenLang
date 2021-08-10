#include "CompilerContext.hpp"

namespace sharpsenLang
{
	IdentifierInfo::IdentifierInfo(TypeHandle typeId, size_t index, IdentifierScope scope)
		: _typeId(typeId),
		  _index(index),
		  _scope(scope)
	{
	}

	TypeHandle IdentifierInfo::typeId() const
	{
		return _typeId;
	}

	size_t IdentifierInfo::index() const
	{
		return _index;
	}

	IdentifierScope IdentifierInfo::getScope() const
	{
		return _scope;
	}

	ClassInfo::ClassInfo(TypeHandle typeId, size_t index, IdentifierScope scope, std::vector<std::string> properties)
		: IdentifierInfo(typeId, index, scope)
	{
		size_t i = 0;
		for (auto &property : properties)
		{
			_propertiesMap.insert({property, i++});
		}
	}

	const size_t *ClassInfo::getPropertyIndex(const std::string &name) const
	{
		if (auto it = _propertiesMap.find(name); it != _propertiesMap.end())
		{
			return &it->second;
		}
		else
		{
			return nullptr;
		}
	}

	const ClassInfo *ClassLookup::createClass(std::string name,
											  TypeHandle typeId, std::vector<std::string> properties)
	{
		return &_identifiers.emplace(std::move(name), ClassInfo(typeId, identifiersSize(), IdentifierScope::Class, std::move(properties))).first->second;
	}

	size_t ClassLookup::identifiersSize() const
	{
		return _identifiers.size();
	}

	const ClassInfo *ClassLookup::find(const std::string &name) const
	{
		if (auto it = _identifiers.find(name); it != _identifiers.end())
		{
			return &it->second;
		}
		else
		{
			return nullptr;
		}
	}

	bool ClassLookup::canDeclare(const std::string &name) const
	{
		return _identifiers.find(name) == _identifiers.end();
	}

	const IdentifierInfo *IdentifierLookup::insertIdentifier(std::string name, TypeHandle typeId, size_t index, IdentifierScope scope)
	{
		return &_identifiers.emplace(std::move(name), IdentifierInfo(typeId, index, scope)).first->second;
	}

	size_t IdentifierLookup::identifiersSize() const
	{
		return _identifiers.size();
	}

	const IdentifierInfo *IdentifierLookup::find(const std::string &name) const
	{
		if (auto it = _identifiers.find(name); it != _identifiers.end())
		{
			return &it->second;
		}
		else
		{
			return nullptr;
		}
	}

	bool IdentifierLookup::canDeclare(const std::string &name) const
	{
		return _identifiers.find(name) == _identifiers.end();
	}

	IdentifierLookup::~IdentifierLookup()
	{
	}

	const IdentifierInfo *GlobalVariableLookup::createIdentifier(std::string name, TypeHandle typeId)
	{
		return insertIdentifier(std::move(name), typeId, identifiersSize(), IdentifierScope::GlobalVariable);
	}

	LocalVariableLookup::LocalVariableLookup(std::unique_ptr<LocalVariableLookup> parent_lookup)
		: _parent(std::move(parent_lookup)),
		  _nextIdentifierIndex(_parent ? _parent->_nextIdentifierIndex : 1)
	{
	}

	const IdentifierInfo *LocalVariableLookup::find(const std::string &name) const
	{
		if (const IdentifierInfo *ret = IdentifierLookup::find(name))
		{
			return ret;
		}
		else
		{
			return _parent ? _parent->find(name) : nullptr;
		}
	}

	const IdentifierInfo *LocalVariableLookup::createIdentifier(std::string name, TypeHandle typeId)
	{
		return insertIdentifier(std::move(name), typeId, _nextIdentifierIndex++, IdentifierScope::LocalVariable);
	}

	std::unique_ptr<LocalVariableLookup> LocalVariableLookup::detachParent()
	{
		return std::move(_parent);
	}

	ParamLookup::ParamLookup()
		: LocalVariableLookup(nullptr),
		  _nextParamIndex(-1)
	{
	}

	const IdentifierInfo *ParamLookup::createParam(std::string name, TypeHandle typeId)
	{
		return insertIdentifier(std::move(name), typeId, _nextParamIndex--, IdentifierScope::LocalVariable);
	}

	const IdentifierInfo *FunctionLookup::createIdentifier(std::string name, TypeHandle typeId)
	{
		return insertIdentifier(std::move(name), typeId, identifiersSize(), IdentifierScope::Function);
	}

	CompilerContext::CompilerContext()
		: _params(nullptr)
	{
	}

	const Type *CompilerContext::getHandle(const Type &t)
	{
		return _types.getHandle(t);
	}

	const IdentifierInfo *CompilerContext::find(const std::string &name) const
	{
		if (_locals)
		{
			if (const IdentifierInfo *ret = _locals->find(name))
			{
				return ret;
			}
		}
		if (const IdentifierInfo *ret = _functions.find(name))
		{
			return ret;
		}
		return _globals.find(name);
	}

	const IdentifierInfo *CompilerContext::findClass(const std::string &name) const
	{
		return _classes.find(name);
	}

	const Property *CompilerContext::getClassProperty(const ClassType *ct, std::string_view propertyName) const
	{
		return _types.getClassProperty(ct, propertyName);
	}

	const IdentifierInfo *CompilerContext::createIdentifier(std::string name, TypeHandle typeId)
	{
		if (_locals)
		{
			return _locals->createIdentifier(std::move(name), typeId);
		}
		else
		{
			return _globals.createIdentifier(std::move(name), typeId);
		}
	}

	const IdentifierInfo *CompilerContext::createParam(std::string name, TypeHandle typeId)
	{
		return _params->createParam(name, typeId);
	}

	const IdentifierInfo *CompilerContext::createFunction(std::string name, TypeHandle typeId)
	{
		return _functions.createIdentifier(name, typeId);
	}

	const IdentifierInfo *CompilerContext::createClass(std::string name, TypeHandle typeId, std::vector<std::string> properties)
	{
		return _classes.createClass(name, typeId, std::move(properties));
	}

	void CompilerContext::enterScope()
	{
		_locals = std::make_unique<LocalVariableLookup>(std::move(_locals));
	}

	void CompilerContext::enterFunction()
	{
		std::unique_ptr<ParamLookup> params = std::make_unique<ParamLookup>();
		_params = params.get();
		_locals = std::move(params);
	}

	void CompilerContext::leaveScope()
	{
		if (_params == _locals.get())
		{
			_params = nullptr;
		}

		_locals = _locals->detachParent();
	}

	bool CompilerContext::canDeclare(const std::string &name) const
	{
		return _locals ? _locals->canDeclare(name) : (_globals.canDeclare(name) && _functions.canDeclare(name) && _classes.canDeclare(name));
	}

	CompilerContext::ScopeRaii CompilerContext::scope()
	{
		return ScopeRaii(*this);
	}

	CompilerContext::FunctionRaii CompilerContext::function()
	{
		return FunctionRaii(*this);
	}

	CompilerContext::ScopeRaii::ScopeRaii(CompilerContext &context)
		: _context(context)
	{
		_context.enterScope();
	}

	CompilerContext::ScopeRaii::~ScopeRaii()
	{
		_context.leaveScope();
	}

	CompilerContext::FunctionRaii::FunctionRaii(CompilerContext &context)
		: _context(context)
	{
		_context.enterFunction();
	}

	CompilerContext::FunctionRaii::~FunctionRaii()
	{
		_context.leaveScope();
	}
}
