#pragma once
#include <memory>
#include <vector>
#include <unordered_map>

#include "Expression.hpp"

namespace sharpsenLang {
	enum struct flow_type{
		f_normal,
		f_break,
		f_continue,
		f_return,
	};
	
	class flow {
	private:
		flow_type _type;
		int _break_level;
		flow(flow_type type, int break_level);
	public:
		flow_type type() const;
		int break_level() const;
		
		static flow normal_flow();
		static flow break_flow(int break_level);
		static flow continue_flow();
		static flow return_flow();
		flow consume_break();
	};
	
	class RuntimeContext;
	
	class statement {
		statement(const statement&) = delete;
		void operator=(const statement&) = delete;
	protected:
		statement() = default;
	public:
		virtual flow execute(RuntimeContext& context) = 0;
		virtual ~statement() = default;
	};
	
	using statement_ptr = std::unique_ptr<statement>;
	using shared_statement_ptr = std::shared_ptr<statement>;
	
	statement_ptr create_simple_statement(Expression<void>::Ptr expr);
	
	statement_ptr create_local_declaration_statement(std::vector<Expression<Lvalue>::Ptr> decls);
	
	statement_ptr create_block_statement(std::vector<statement_ptr> statements);
	shared_statement_ptr create_shared_block_statement(std::vector<statement_ptr> statements);
	
	statement_ptr create_break_statement(int break_level);
	
	statement_ptr create_continue_statement();
	
	statement_ptr create_return_statement(Expression<Lvalue>::Ptr expr);
	
	statement_ptr create_return_void_statement();
	
	statement_ptr create_if_statement(
		std::vector<Expression<Lvalue>::Ptr> decls,
		std::vector<Expression<Number>::Ptr> exprs,
		std::vector<statement_ptr> statements
	);
	
	statement_ptr create_switch_statement(
		std::vector<Expression<Lvalue>::Ptr> decls,
		Expression<Number>::Ptr expr,
		std::vector<statement_ptr> statements,
		std::unordered_map<Number, size_t> cases,
		size_t dflt
	);
	
	
	statement_ptr create_while_statement(Expression<Number>::Ptr expr, statement_ptr statement);
	
	statement_ptr create_do_statement(Expression<Number>::Ptr expr, statement_ptr statement);
	
	statement_ptr create_for_statement(
		Expression<Void>::Ptr expr1,
		Expression<Number>::Ptr expr2,
		Expression<Void>::Ptr expr3,
		statement_ptr statement
	);
	
	statement_ptr create_for_statement(
		std::vector<Expression<Lvalue>::Ptr> decls,
		Expression<Number>::Ptr expr2,
		Expression<Void>::Ptr expr3,
		statement_ptr statement
	);
}

