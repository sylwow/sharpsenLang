#pragma once
#include <memory>
#include <vector>
#include <unordered_map>

#include "Expression.hpp"

namespace sharpsenLang
{
	enum struct FlowType
	{
		FlowNormal,
		FlowBreak,
		FlowContinue,
		FlowReturn,
	};

	class Flow
	{
	private:
		FlowType _type;
		int _breakLevel;
		Flow(FlowType type, int breakLevel);

	public:
		FlowType type() const;
		int breakLevel() const;

		static Flow normalFlow();
		static Flow breakFlow(int breakLevel);
		static Flow continueFlow();
		static Flow returnFlow();
		Flow consumeBreak();
	};

	class RuntimeContext;

	class Statement
	{
		Statement(const Statement &) = delete;
		void operator=(const Statement &) = delete;

	protected:
		Statement() = default;

	public:
		virtual Flow execute(RuntimeContext &context) = 0;
		virtual ~Statement() = default;
	};

	using StatementPtr = std::unique_ptr<Statement>;
	using SharedStatementPtr = std::shared_ptr<Statement>;

	StatementPtr createSimpleStatement(Expression<void>::Ptr expr);

	StatementPtr createLocalDeclarationStatement(std::vector<Expression<Lvalue>::Ptr> decls);

	StatementPtr createBlockStatement(std::vector<StatementPtr> statements);
	SharedStatementPtr createSharedBlockStatement(std::vector<StatementPtr> statements);

	StatementPtr createBreakStatement(int breakLevel);

	StatementPtr createContinueStatement();

	StatementPtr createReturnStatement(Expression<Lvalue>::Ptr expr);

	StatementPtr createReturnVoidStatement();

	StatementPtr createIfStatement(
		std::vector<Expression<Lvalue>::Ptr> decls,
		std::vector<Expression<Number>::Ptr> exprs,
		std::vector<StatementPtr> statements);

	StatementPtr createSwitchStatement(
		std::vector<Expression<Lvalue>::Ptr> decls,
		Expression<Number>::Ptr expr,
		std::vector<StatementPtr> statements,
		std::unordered_map<Number, size_t> cases,
		size_t dflt);

	StatementPtr createWhileStatement(Expression<Number>::Ptr expr, StatementPtr statement);

	StatementPtr createDoStatement(Expression<Number>::Ptr expr, StatementPtr statement);

	StatementPtr createForStatement(
		Expression<Void>::Ptr expr1,
		Expression<Number>::Ptr expr2,
		Expression<Void>::Ptr expr3,
		StatementPtr statement);

	StatementPtr createForStatement(
		std::vector<Expression<Lvalue>::Ptr> decls,
		Expression<Number>::Ptr expr2,
		Expression<Void>::Ptr expr3,
		StatementPtr statement);
}
