#pragma once
#include <variant>
#include <vector>
#include <deque>
#include <stack>
#include <string>
#include <unordered_map>

#include "Variable.hpp"
#include "Lookup.hpp"
#include "Expression.hpp"

namespace sharpsenLang
{
	class RuntimeContext
	{
	private:
		std::vector<Function> _functions;
		std::unordered_map<std::string, size_t> _publicFunctions;
		std::vector<Expression<Lvalue>::Ptr> _initializers;
		std::vector<VariablePtr> _globals;
		std::deque<VariablePtr> _stack;
		size_t _retvalIdx;

		class scope
		{
		private:
			RuntimeContext &_context;
			size_t _stackSize;

		public:
			scope(RuntimeContext &context);
			~scope();
		};

	public:
		RuntimeContext(
			std::vector<Expression<Lvalue>::Ptr> initializers,
			std::vector<Function> functions,
			std::unordered_map<std::string, size_t> publicFunctions);

		void initialize();

		VariablePtr &global(int idx);
		VariablePtr &retval();
		VariablePtr &local(int idx);

		const Function &getFunction(int idx) const;
		const Function &getPublicFunction(const char *name) const;

		scope enterScope();
		void push(VariablePtr v);

		VariablePtr call(const Function &f, std::vector<VariablePtr> params);
	};
}
