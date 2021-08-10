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
		Param,

		Preinc,
		Predec,
		Postinc,
		Postdec,
		Positive,
		Negative,
		Bnot,
		Lnot,
		Size,
		ToString,

		Add,
		Sub,
		Mul,
		Div,
		Idiv,
		Mod,
		Band,
		Bor,
		Bxor,
		Bsl,
		Bsr,
		Concat,
		Assign,
		AddAssign,
		SubAssign,
		MulAssign,
		DivAssign,
		IdivAssign,
		ModAssign,
		BandAssign,
		BorAssign,
		BxorAssign,
		BslAssign,
		BsrAssign,
		ConcatAssign,
		Eq,
		Ne,
		Lt,
		Gt,
		Le,
		Ge,
		Comma,
		Land,
		Lor,
		Index,

		Ternary,

		Call,

		Init,

		Get,
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
		Node(CompilerContext &context, NodeValue value, std::vector<NodePtr> children, size_t lineNumber, size_t charIndex, bool canBeUndefined = false);

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

		void checkConversion(TypeHandle typeId, bool lvalue) const;
	};

}
