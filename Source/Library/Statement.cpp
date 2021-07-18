#include <unordered_map>

#include "Statement.hpp"
#include "Expression.hpp"
#include "RuntimeContext.hpp"

namespace sharpsenLang
{
	Flow::Flow(FlowType type, int breakLevel) : _type(type),
												_breakLevel(breakLevel)
	{
	}

	FlowType Flow::type() const
	{
		return _type;
	}

	int Flow::breakLevel() const
	{
		return _breakLevel;
	}

	Flow Flow::normalFlow()
	{
		return Flow(FlowType::FlowNormal, 0);
	}

	Flow Flow::breakFlow(int breakLevel)
	{
		return Flow(FlowType::FlowBreak, breakLevel);
	}

	Flow Flow::continueFlow()
	{
		return Flow(FlowType::FlowContinue, 0);
	}

	Flow Flow::returnFlow()
	{
		return Flow(FlowType::FlowReturn, 0);
	}

	Flow Flow::consumeBreak()
	{
		return _breakLevel == 1 ? Flow::normalFlow() : Flow::breakFlow(_breakLevel - 1);
	}

	namespace
	{
		class SimpleStatement : public Statement
		{
		private:
			Expression<Void>::Ptr _expr;

		public:
			SimpleStatement(Expression<void>::Ptr expr) : _expr(std::move(expr))
			{
			}

			Flow execute(RuntimeContext &context) override
			{
				_expr->evaluate(context);
				return Flow::normalFlow();
			}
		};

		class BlockStatement : public Statement
		{
		private:
			std::vector<StatementPtr> _statements;

		public:
			BlockStatement(std::vector<StatementPtr> statements) : _statements(std::move(statements))
			{
			}

			Flow execute(RuntimeContext &context) override
			{
				auto _ = context.enterScope();
				for (const StatementPtr &statement : _statements)
				{
					if (Flow f = statement->execute(context); f.type() != FlowType::FlowNormal)
					{
						return f;
					}
				}
				return Flow::normalFlow();
			}
		};

		class LocalDeclarationStatement : public Statement
		{
		private:
			std::vector<Expression<Lvalue>::Ptr> _decls;

		public:
			LocalDeclarationStatement(std::vector<Expression<Lvalue>::Ptr> decls) : _decls(std::move(decls))
			{
			}

			Flow execute(RuntimeContext &context) override
			{
				for (const Expression<Lvalue>::Ptr &decl : _decls)
				{
					context.push(decl->evaluate(context));
				}
				return Flow::normalFlow();
			}
		};

		class BreakStatement : public Statement
		{
		private:
			int _breakLevel;

		public:
			BreakStatement(int breakLevel) : _breakLevel(breakLevel)
			{
			}

			Flow execute(RuntimeContext &) override
			{
				return Flow::breakFlow(_breakLevel);
			}
		};

		class ContinueStatement : public Statement
		{
		public:
			ContinueStatement() = default;

			Flow execute(RuntimeContext &) override
			{
				return Flow::continueFlow();
			}
		};

		class ReturnStatement : public Statement
		{
		private:
			Expression<Lvalue>::Ptr _expr;

		public:
			ReturnStatement(Expression<Lvalue>::Ptr expr) : _expr(std::move(expr))
			{
			}

			Flow execute(RuntimeContext &context) override
			{
				context.retval() = _expr->evaluate(context);
				return Flow::returnFlow();
			}
		};

		class ReturnVoidStatement : public Statement
		{
		public:
			ReturnVoidStatement() = default;

			Flow execute(RuntimeContext &) override
			{
				return Flow::returnFlow();
			}
		};

		class IfStatement : public Statement
		{
		private:
			std::vector<Expression<Number>::Ptr> _exprs;
			std::vector<StatementPtr> _statements;

		public:
			IfStatement(std::vector<Expression<Number>::Ptr> exprs, std::vector<StatementPtr> statements) : _exprs(std::move(exprs)),
																											_statements(std::move(statements))
			{
			}

			Flow execute(RuntimeContext &context) override
			{
				for (size_t i = 0; i < _exprs.size(); ++i)
				{
					if (_exprs[i]->evaluate(context))
					{
						return _statements[i]->execute(context);
					}
				}
				return _statements.back()->execute(context);
			}
		};

		class IfDeclareStatement : public IfStatement
		{
		private:
			std::vector<Expression<Lvalue>::Ptr> _decls;

		public:
			IfDeclareStatement(
				std::vector<Expression<Lvalue>::Ptr> decls,
				std::vector<Expression<Number>::Ptr> exprs,
				std::vector<StatementPtr> statements) : IfStatement(std::move(exprs), std::move(statements)),
														_decls(std::move(decls))
			{
			}

			Flow execute(RuntimeContext &context) override
			{
				auto _ = context.enterScope();

				for (const Expression<Lvalue>::Ptr &decl : _decls)
				{
					context.push(decl->evaluate(context));
				}

				return IfStatement::execute(context);
			}
		};

		class SwitchStatement : public Statement
		{
		private:
			Expression<Number>::Ptr _expr;
			std::vector<StatementPtr> _statements;
			std::unordered_map<Number, size_t> _cases;
			size_t _dflt;

		public:
			SwitchStatement(
				Expression<Number>::Ptr expr,
				std::vector<StatementPtr> statements,
				std::unordered_map<Number, size_t> cases,
				size_t dflt) : _expr(std::move(expr)),
							   _statements(std::move(statements)),
							   _cases(std::move(cases)),
							   _dflt(dflt)
			{
			}

			Flow execute(RuntimeContext &context) override
			{
				auto it = _cases.find(_expr->evaluate(context));
				for (size_t idx = (it == _cases.end() ? _dflt : it->second); idx < _statements.size(); ++idx)
				{
					switch (Flow f = _statements[idx]->execute(context); f.type())
					{
					case FlowType::FlowNormal:
						break;
					case FlowType::FlowBreak:
						return f.consumeBreak();
					default:
						return f;
					}
				}

				return Flow::normalFlow();
			}
		};

		class SwitchDeclareStatement : public SwitchStatement
		{
		private:
			std::vector<Expression<Lvalue>::Ptr> _decls;

		public:
			SwitchDeclareStatement(
				std::vector<Expression<Lvalue>::Ptr> decls,
				Expression<Number>::Ptr expr,
				std::vector<StatementPtr> statements,
				std::unordered_map<Number, size_t> cases,
				size_t dflt) : _decls(std::move(decls)),
							   SwitchStatement(std::move(expr), std::move(statements), std::move(cases), dflt)
			{
			}

			Flow execute(RuntimeContext &context) override
			{
				auto _ = context.enterScope();

				for (const Expression<Lvalue>::Ptr &decl : _decls)
				{
					context.push(decl->evaluate(context));
				}

				return SwitchStatement::execute(context);
			}
		};

		class WhileStatement : public Statement
		{
		private:
			Expression<Number>::Ptr _expr;
			StatementPtr _statement;

		public:
			WhileStatement(Expression<Number>::Ptr expr, StatementPtr statement) : _expr(std::move(expr)),
																				   _statement(std::move(statement))
			{
			}

			Flow execute(RuntimeContext &context) override
			{
				while (_expr->evaluate(context))
				{
					switch (Flow f = _statement->execute(context); f.type())
					{
					case FlowType::FlowNormal:
					case FlowType::FlowContinue:
						break;
					case FlowType::FlowBreak:
						return f.consumeBreak();
					case FlowType::FlowReturn:
						return f;
					}
				}

				return Flow::normalFlow();
			}
		};

		class DoStatement : public Statement
		{
		private:
			Expression<Number>::Ptr _expr;
			StatementPtr _statement;

		public:
			DoStatement(Expression<Number>::Ptr expr, StatementPtr statement) : _expr(std::move(expr)),
																				_statement(std::move(statement))
			{
			}

			Flow execute(RuntimeContext &context) override
			{
				do
				{
					switch (Flow f = _statement->execute(context); f.type())
					{
					case FlowType::FlowNormal:
					case FlowType::FlowContinue:
						break;
					case FlowType::FlowBreak:
						return f.consumeBreak();
					case FlowType::FlowReturn:
						return f;
					}
				} while (_expr->evaluate(context));

				return Flow::normalFlow();
			}
		};

		class ForStatementBase : public Statement
		{
		private:
			Expression<Number>::Ptr _expr2;
			Expression<Void>::Ptr _expr3;
			StatementPtr _statement;

		public:
			ForStatementBase(
				Expression<Number>::Ptr expr2,
				Expression<Void>::Ptr expr3,
				StatementPtr statement) : _expr2(std::move(expr2)),
										  _expr3(std::move(expr3)),
										  _statement(std::move(statement))
			{
			}

			Flow execute(RuntimeContext &context) override
			{
				for (; _expr2->evaluate(context); _expr3->evaluate(context))
				{
					switch (Flow f = _statement->execute(context); f.type())
					{
					case FlowType::FlowNormal:
					case FlowType::FlowContinue:
						break;
					case FlowType::FlowBreak:
						return f.consumeBreak();
					case FlowType::FlowReturn:
						return f;
					}
				}

				return Flow::normalFlow();
			}
		};

		class ForStatement : public ForStatementBase
		{
		private:
			Expression<Void>::Ptr _expr1;

		public:
			ForStatement(
				Expression<Void>::Ptr expr1,
				Expression<Number>::Ptr expr2,
				Expression<Void>::Ptr expr3,
				StatementPtr statement) : ForStatementBase(std::move(expr2), std::move(expr3), std::move(statement)),
										  _expr1(std::move(expr1))
			{
			}

			Flow execute(RuntimeContext &context) override
			{
				_expr1->evaluate(context);

				return ForStatementBase::execute(context);
			}
		};

		class ForDeclareStatement : public ForStatementBase
		{
		private:
			std::vector<Expression<Lvalue>::Ptr> _decls;
			Expression<Number>::Ptr _expr2;
			Expression<Void>::Ptr _expr3;
			StatementPtr _statement;

		public:
			ForDeclareStatement(
				std::vector<Expression<Lvalue>::Ptr> decls,
				Expression<Number>::Ptr expr2,
				Expression<Void>::Ptr expr3,
				StatementPtr statement) : ForStatementBase(std::move(expr2), std::move(expr3), std::move(statement)),
										  _decls(std::move(decls))
			{
			}

			Flow execute(RuntimeContext &context) override
			{
				auto _ = context.enterScope();

				for (const Expression<Lvalue>::Ptr &decl : _decls)
				{
					context.push(decl->evaluate(context));
				}

				return ForStatementBase::execute(context);
			}
		};
	}

	StatementPtr createSimpleStatement(Expression<Void>::Ptr expr)
	{
		return std::make_unique<SimpleStatement>(std::move(expr));
	}

	StatementPtr createLocalDeclarationStatement(std::vector<Expression<Lvalue>::Ptr> decls)
	{
		return std::make_unique<LocalDeclarationStatement>(std::move(decls));
	}

	StatementPtr createBlockStatement(std::vector<StatementPtr> statements)
	{
		return std::make_unique<BlockStatement>(std::move(statements));
	}

	SharedStatementPtr createSharedBlockStatement(std::vector<StatementPtr> statements)
	{
		return std::make_shared<BlockStatement>(std::move(statements));
	}

	StatementPtr createBreakStatement(int breakLevel)
	{
		return std::make_unique<BreakStatement>(breakLevel);
	}

	StatementPtr createContinueStatement()
	{
		return std::make_unique<ContinueStatement>();
	}

	StatementPtr createReturnStatement(Expression<Lvalue>::Ptr expr)
	{
		return std::make_unique<ReturnStatement>(std::move(expr));
	}

	StatementPtr createReturnVoidStatement()
	{
		return std::make_unique<ReturnVoidStatement>();
	}

	StatementPtr createIfStatement(
		std::vector<Expression<Lvalue>::Ptr> decls,
		std::vector<Expression<Number>::Ptr> exprs,
		std::vector<StatementPtr> statements)
	{
		if (!decls.empty())
		{
			return std::make_unique<IfDeclareStatement>(std::move(decls), std::move(exprs), std::move(statements));
		}
		else
		{
			return std::make_unique<IfStatement>(std::move(exprs), std::move(statements));
		}
	}

	StatementPtr createSwitchStatement(
		std::vector<Expression<Lvalue>::Ptr> decls,
		Expression<Number>::Ptr expr,
		std::vector<StatementPtr> statements,
		std::unordered_map<Number, size_t> cases,
		size_t dflt)
	{
		if (!decls.empty())
		{
			return std::make_unique<SwitchDeclareStatement>(
				std::move(decls),
				std::move(expr),
				std::move(statements),
				std::move(cases), dflt);
		}
		else
		{
			return std::make_unique<SwitchStatement>(
				std::move(expr),
				std::move(statements),
				std::move(cases), dflt);
		}
	}

	StatementPtr createWhileStatement(Expression<Number>::Ptr expr, StatementPtr statement)
	{
		return std::make_unique<WhileStatement>(std::move(expr), std::move(statement));
	}

	StatementPtr createDoStatement(Expression<Number>::Ptr expr, StatementPtr statement)
	{
		return std::make_unique<DoStatement>(std::move(expr), std::move(statement));
	}

	StatementPtr createForStatement(
		Expression<Void>::Ptr expr1,
		Expression<Number>::Ptr expr2,
		Expression<Void>::Ptr expr3,
		StatementPtr statement)
	{
		return std::make_unique<ForStatement>(std::move(expr1), std::move(expr2), std::move(expr3), std::move(statement));
	}

	StatementPtr createForStatement(
		std::vector<Expression<Lvalue>::Ptr> decls,
		Expression<Number>::Ptr expr2,
		Expression<Void>::Ptr expr3,
		StatementPtr statement)
	{
		return std::make_unique<ForDeclareStatement>(std::move(decls), std::move(expr2), std::move(expr3), std::move(statement));
	}
}
