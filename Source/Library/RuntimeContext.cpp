#include "RuntimeContext.hpp"
#include "Errors.hpp"

namespace sharpsenLang
{
	RuntimeContext::RuntimeContext(
		std::vector<Expression<Lvalue>::Ptr> initializers,
		std::vector<Function> functions,
		std::unordered_map<std::string, size_t> publicFunctions)
		: _functions(std::move(functions)),
		  _publicFunctions(std::move(publicFunctions)),
		  _initializers(std::move(initializers)),
		  _retvalIdx(0)
	{
		_globals.reserve(_initializers.size());
		initialize();
	}

	void RuntimeContext::initialize()
	{
		_globals.clear();

		for (const auto &initializer : _initializers)
		{
			_globals.emplace_back(initializer->evaluate(*this));
		}
	}

	VariablePtr &RuntimeContext::global(int idx)
	{
		runtimeAssertion(idx < _globals.size(), "Uninitialized global variable access");
		return _globals[idx];
	}

	VariablePtr &RuntimeContext::retval()
	{
		return _stack[_retvalIdx];
	}

	VariablePtr &RuntimeContext::local(int idx)
	{
		return _stack[_retvalIdx + idx];
	}

	const Function &RuntimeContext::getFunction(int idx) const
	{
		return _functions[idx];
	}

	const Function &RuntimeContext::getPublicFunction(const char *name) const
	{
		return _functions[_publicFunctions.find(name)->second];
	}

	RuntimeContext::scope RuntimeContext::enterScope()
	{
		return scope(*this);
	}

	void RuntimeContext::push(VariablePtr v)
	{
		_stack.push_back(std::move(v));
	}

	VariablePtr RuntimeContext::call(const Function &f, std::vector<VariablePtr> params)
	{
		for (size_t i = params.size(); i > 0; --i)
		{
			_stack.push_back(std::move(params[i - 1]));
		}
		size_t old_retval_idx = _retvalIdx;

		_retvalIdx = _stack.size();
		_stack.resize(_retvalIdx + 1);

		runtimeAssertion(bool(f), "Uninitialized Function call");

		f(*this);

		VariablePtr ret = std::move(_stack[_retvalIdx]);

		_stack.resize(_retvalIdx - params.size());

		_retvalIdx = old_retval_idx;

		return ret;
	}

	RuntimeContext::scope::scope(RuntimeContext &context)
		: _context(context),
		  _stackSize(context._stack.size())
	{
	}

	RuntimeContext::scope::~scope()
	{
		_context._stack.resize(_stackSize);
	}
}
