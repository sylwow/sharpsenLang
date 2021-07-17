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
		return insertIdentifier(std::move(name), typeId, identifiersSize(), IdentifierScope::global_variable);
	}

	LocalVariableLookup::LocalVariableLookup(std::unique_ptr<LocalVariableLookup> parent_lookup) : _parent(std::move(parent_lookup)),
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
		return insertIdentifier(std::move(name), typeId, _nextIdentifierIndex++, IdentifierScope::local_variable);
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
		return insertIdentifier(std::move(name), typeId, _nextParamIndex--, IdentifierScope::local_variable);
	}

	const IdentifierInfo *FunctionLookup::createIdentifier(std::string name, TypeHandle typeId)
	{
		return insertIdentifier(std::move(name), typeId, identifiersSize(), IdentifierScope::function);
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
		return _locals ? _locals->canDeclare(name) : (_globals.canDeclare(name) && _functions.canDeclare(name));
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
