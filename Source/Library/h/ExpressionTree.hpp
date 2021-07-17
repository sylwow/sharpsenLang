#pragma once
#include <memory>
#include <variant>
#include <vector>

#include "Tokens.hpp"
#include "Types.hpp"

namespace sharpsenLang
{

	enum struct NodeOperation
	{
		param,

		preinc,
		predec,
		postinc,
		postdec,
		positive,
		negative,
		bnot,
		lnot,
		size,
		tostring,

		add,
		sub,
		mul,
		div,
		idiv,
		mod,
		band,
		bor,
		bxor,
		bsl,
		bsr,
		concat,
		assign,
		add_assign,
		sub_assign,
		mul_assign,
		div_assign,
		idiv_assign,
		mod_assign,
		band_assign,
		bor_assign,
		bxor_assign,
		bsl_assign,
		bsr_assign,
		concat_assign,
		eq,
		ne,
		lt,
		gt,
		le,
		ge,
		comma,
		land,
		lor,
		index,

		ternary,

		call,

		init,
	};

	struct Node;
	using NodePtr = std::unique_ptr<Node>;

	using NodeValue = std::variant<NodeOperation, std::string, double, Identifier>;

	class CompilerContext;

	struct Node
	{
	private:
		NodeValue _value;
		std::vector<NodePtr> _children;
		TypeHandle _typeId;
		bool _lvalue;
		size_t _lineNumber;
		size_t _charIndex;

	public:
		Node(CompilerContext &context, NodeValue value, std::vector<NodePtr> children, size_t lineNumber, size_t charIndex);

		const NodeValue &getValue() const;

		bool isNodeOperation() const;
		bool isIdentifier() const;
		bool isNumber() const;
		bool isString() const;

		NodeOperation getNodeOperation() const;
		std::string_view getIdentifier() const;
		double getNumber() const;
		std::string_view getString() const;

		const std::vector<NodePtr> &getChildren() const;

		TypeHandle getTypeId() const;
		bool isLvalue() const;

		size_t getLineNumber() const;
		size_t getCharIndex() const;

		void checkConversion(TypeHandle type_id, bool lvalue) const;
	};

}
