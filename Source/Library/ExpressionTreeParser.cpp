#include <stack>

#include "ExpressionTreeParser.hpp"
#include "ExpressionTree.hpp"
#include "Tokenizer.hpp"
#include "Tokens.hpp"
#include "Errors.hpp"

namespace sharpsenLang
{
	namespace
	{
		enum struct OperatorPrecedence
		{
			Brackets,
			Postfix,
			Prefix,
			Multiplication,
			Addition,
			Shift,
			Comparison,
			Equality,
			BitwiseAnd,
			BitwiseXor,
			BitwiseOr,
			LogicalAnd,
			LogicalOr,
			Assignment,
			Comma
		};

		enum struct OperatorAssociativity
		{
			LeftToRight,
			RightToLeft
		};

		struct OperatorInfo
		{
			NodeOperation operation;
			OperatorPrecedence precedence;
			OperatorAssociativity associativity;
			int number_of_operands;
			size_t lineNumber;
			size_t charIndex;

			OperatorInfo(NodeOperation operation, size_t lineNumber, size_t charIndex)
				: operation(operation),
				  lineNumber(lineNumber),
				  charIndex(charIndex)
			{
				switch (operation)
				{
				case NodeOperation::Init:
					precedence = OperatorPrecedence::Brackets;
					break;
				case NodeOperation::Param: // This will never happen. Used only for the Node creation.
				case NodeOperation::Postinc:
				case NodeOperation::Postdec:
				case NodeOperation::Index:
				case NodeOperation::Call:
					precedence = OperatorPrecedence::Postfix;
					break;
				case NodeOperation::Preinc:
				case NodeOperation::Predec:
				case NodeOperation::Positive:
				case NodeOperation::Negative:
				case NodeOperation::Bnot:
				case NodeOperation::Lnot:
				case NodeOperation::Size:
				case NodeOperation::ToString:
					precedence = OperatorPrecedence::Prefix;
					break;
				case NodeOperation::Mul:
				case NodeOperation::Div:
				case NodeOperation::Idiv:
				case NodeOperation::Mod:
					precedence = OperatorPrecedence::Multiplication;
					break;
				case NodeOperation::Add:
				case NodeOperation::Sub:
				case NodeOperation::Concat:
					precedence = OperatorPrecedence::Addition;
					break;
				case NodeOperation::Bsl:
				case NodeOperation::Bsr:
					precedence = OperatorPrecedence::Shift;
					break;
				case NodeOperation::Lt:
				case NodeOperation::Gt:
				case NodeOperation::Le:
				case NodeOperation::Ge:
					precedence = OperatorPrecedence::Comparison;
					break;
				case NodeOperation::Eq:
				case NodeOperation::Ne:
					precedence = OperatorPrecedence::Equality;
					break;
				case NodeOperation::Band:
					precedence = OperatorPrecedence::BitwiseAnd;
					break;
				case NodeOperation::Bxor:
					precedence = OperatorPrecedence::BitwiseXor;
					break;
				case NodeOperation::Bor:
					precedence = OperatorPrecedence::BitwiseOr;
					break;
				case NodeOperation::Land:
					precedence = OperatorPrecedence::LogicalAnd;
					break;
				case NodeOperation::Lor:
					precedence = OperatorPrecedence::LogicalOr;
					break;
				case NodeOperation::Assign:
				case NodeOperation::AddAssign:
				case NodeOperation::SubAssign:
				case NodeOperation::MulAssign:
				case NodeOperation::DivAssign:
				case NodeOperation::IdivAssign:
				case NodeOperation::ModAssign:
				case NodeOperation::BandAssign:
				case NodeOperation::BorAssign:
				case NodeOperation::BxorAssign:
				case NodeOperation::BslAssign:
				case NodeOperation::BsrAssign:
				case NodeOperation::ConcatAssign:
				case NodeOperation::Ternary:
					precedence = OperatorPrecedence::Assignment;
					break;
				case NodeOperation::Comma:
					precedence = OperatorPrecedence::Comma;
					break;
				}

				switch (precedence)
				{
				case OperatorPrecedence::Prefix:
				case OperatorPrecedence::Assignment:
					associativity = OperatorAssociativity::RightToLeft;
					break;
				default:
					associativity = OperatorAssociativity::LeftToRight;
					break;
				}

				switch (operation)
				{
				case NodeOperation::Init:
					number_of_operands = 0; //zero or more
					break;
				case NodeOperation::Postinc:
				case NodeOperation::Postdec:
				case NodeOperation::Preinc:
				case NodeOperation::Predec:
				case NodeOperation::Positive:
				case NodeOperation::Negative:
				case NodeOperation::Bnot:
				case NodeOperation::Lnot:
				case NodeOperation::Size:
				case NodeOperation::ToString:
				case NodeOperation::Call: //at least one
					number_of_operands = 1;
					break;
				case NodeOperation::Ternary:
					number_of_operands = 3;
					break;
				default:
					number_of_operands = 2;
					break;
				}
			}
		};

		OperatorInfo getOperatorInfo(ReservedToken token, bool prefix, size_t lineNumber, size_t charIndex)
		{
			switch (token)
			{
			case ReservedToken::Inc:
				return prefix ? OperatorInfo(NodeOperation::Preinc, lineNumber, charIndex)
							  : OperatorInfo(NodeOperation::Postinc, lineNumber, charIndex);
			case ReservedToken::Dec:
				return prefix ? OperatorInfo(NodeOperation::Predec, lineNumber, charIndex)
							  : OperatorInfo(NodeOperation::Postdec, lineNumber, charIndex);
			case ReservedToken::Add:
				return prefix ? OperatorInfo(NodeOperation::Positive, lineNumber, charIndex)
							  : OperatorInfo(NodeOperation::Add, lineNumber, charIndex);
			case ReservedToken::Sub:
				return prefix ? OperatorInfo(NodeOperation::Negative, lineNumber, charIndex)
							  : OperatorInfo(NodeOperation::Sub, lineNumber, charIndex);
			case ReservedToken::Concat:
				return OperatorInfo(NodeOperation::Concat, lineNumber, charIndex);
			case ReservedToken::Mul:
				return OperatorInfo(NodeOperation::Mul, lineNumber, charIndex);
			case ReservedToken::Div:
				return OperatorInfo(NodeOperation::Div, lineNumber, charIndex);
			case ReservedToken::Idiv:
				return OperatorInfo(NodeOperation::Idiv, lineNumber, charIndex);
			case ReservedToken::Mod:
				return OperatorInfo(NodeOperation::Mod, lineNumber, charIndex);
			case ReservedToken::BitwiseNot:
				return OperatorInfo(NodeOperation::Bnot, lineNumber, charIndex);
			case ReservedToken::BitwiseAnd:
				return OperatorInfo(NodeOperation::Band, lineNumber, charIndex);
			case ReservedToken::BitwiseOr:
				return OperatorInfo(NodeOperation::Bor, lineNumber, charIndex);
			case ReservedToken::BitwiseXor:
				return OperatorInfo(NodeOperation::Bxor, lineNumber, charIndex);
			case ReservedToken::Shiftl:
				return OperatorInfo(NodeOperation::Bsl, lineNumber, charIndex);
			case ReservedToken::Shiftr:
				return OperatorInfo(NodeOperation::Bsr, lineNumber, charIndex);
			case ReservedToken::Assign:
				return OperatorInfo(NodeOperation::Assign, lineNumber, charIndex);
			case ReservedToken::AddAssign:
				return OperatorInfo(NodeOperation::AddAssign, lineNumber, charIndex);
			case ReservedToken::SubAssign:
				return OperatorInfo(NodeOperation::SubAssign, lineNumber, charIndex);
			case ReservedToken::ConcatAssign:
				return OperatorInfo(NodeOperation::ConcatAssign, lineNumber, charIndex);
			case ReservedToken::MulAssign:
				return OperatorInfo(NodeOperation::ModAssign, lineNumber, charIndex);
			case ReservedToken::DivAssign:
				return OperatorInfo(NodeOperation::DivAssign, lineNumber, charIndex);
			case ReservedToken::IdivAssign:
				return OperatorInfo(NodeOperation::IdivAssign, lineNumber, charIndex);
			case ReservedToken::ModAssign:
				return OperatorInfo(NodeOperation::ModAssign, lineNumber, charIndex);
			case ReservedToken::AndAssign:
				return OperatorInfo(NodeOperation::BandAssign, lineNumber, charIndex);
			case ReservedToken::OrAssign:
				return OperatorInfo(NodeOperation::BorAssign, lineNumber, charIndex);
			case ReservedToken::XorAssign:
				return OperatorInfo(NodeOperation::BxorAssign, lineNumber, charIndex);
			case ReservedToken::ShiftlAssign:
				return OperatorInfo(NodeOperation::BslAssign, lineNumber, charIndex);
			case ReservedToken::ShiftrAssign:
				return OperatorInfo(NodeOperation::BsrAssign, lineNumber, charIndex);
			case ReservedToken::LogicalNot:
				return OperatorInfo(NodeOperation::Lnot, lineNumber, charIndex);
			case ReservedToken::LogicalAnd:
				return OperatorInfo(NodeOperation::Land, lineNumber, charIndex);
			case ReservedToken::LogicalOr:
				return OperatorInfo(NodeOperation::Lor, lineNumber, charIndex);
			case ReservedToken::Eq:
				return OperatorInfo(NodeOperation::Eq, lineNumber, charIndex);
			case ReservedToken::Ne:
				return OperatorInfo(NodeOperation::Ne, lineNumber, charIndex);
			case ReservedToken::Lt:
				return OperatorInfo(NodeOperation::Lt, lineNumber, charIndex);
			case ReservedToken::Gt:
				return OperatorInfo(NodeOperation::Gt, lineNumber, charIndex);
			case ReservedToken::Le:
				return OperatorInfo(NodeOperation::Le, lineNumber, charIndex);
			case ReservedToken::Ge:
				return OperatorInfo(NodeOperation::Ge, lineNumber, charIndex);
			case ReservedToken::Question:
				return OperatorInfo(NodeOperation::Ternary, lineNumber, charIndex);
			case ReservedToken::Comma:
				return OperatorInfo(NodeOperation::Comma, lineNumber, charIndex);
			case ReservedToken::OpenRound:
				return OperatorInfo(NodeOperation::Call, lineNumber, charIndex);
			case ReservedToken::OpenSquare:
				return OperatorInfo(NodeOperation::Index, lineNumber, charIndex);
			case ReservedToken::KwSizeof:
				return OperatorInfo(NodeOperation::Size, lineNumber, charIndex);
			case ReservedToken::KwToString:
				return OperatorInfo(NodeOperation::ToString, lineNumber, charIndex);
			case ReservedToken::OpenCurly:
				return OperatorInfo(NodeOperation::Init, lineNumber, charIndex);
			default:
				throw unexpectedSyntaxError(std::to_string(token), lineNumber, charIndex);
			}
		}

		bool isEndOfExpression(const Token &t, bool allowComma)
		{
			if (t.isEof())
			{
				return true;
			}

			if (t.isReservedToken())
			{
				switch (t.getReservedToken())
				{
				case ReservedToken::Semicolon:
				case ReservedToken::CloseRound:
				case ReservedToken::CloseSquare:
				case ReservedToken::CloseCurly:
				case ReservedToken::Colon:
					return true;
				case ReservedToken::Comma:
					return !allowComma;
				default:
					return false;
				}
			}

			return false;
		}

		bool isEvaluatedBefore(const OperatorInfo &l, const OperatorInfo &r)
		{
			return l.associativity == OperatorAssociativity::LeftToRight ? l.precedence <= r.precedence : l.precedence < r.precedence;
		}

		void popOneOperator(
			std::stack<OperatorInfo> &operatorStack, std::stack<NodePtr> &operandStack,
			CompilerContext &context, size_t lineNumber, size_t charIndex)
		{
			if (operandStack.size() < operatorStack.top().number_of_operands)
			{
				throw compilerError("Failed to parse an expression", lineNumber, charIndex);
			}

			std::vector<NodePtr> operands;
			operands.resize(operatorStack.top().number_of_operands);

			if (operatorStack.top().precedence != OperatorPrecedence::Prefix)
			{
				operatorStack.top().lineNumber = operandStack.top()->getLineNumber();
				operatorStack.top().charIndex = operandStack.top()->getCharIndex();
			}

			for (int i = operatorStack.top().number_of_operands - 1; i >= 0; --i)
			{
				operands[i] = std::move(operandStack.top());
				operandStack.pop();
			}

			operandStack.push(std::make_unique<Node>(
				context, operatorStack.top().operation, std::move(operands), operatorStack.top().lineNumber, operatorStack.top().charIndex));

			operatorStack.pop();
		}

		NodePtr parseExpressionTreeImpl(CompilerContext &context, TokensIterator &it, bool allowComma, bool allowEmpty)
		{
			std::stack<NodePtr> operandStack;
			std::stack<OperatorInfo> operatorStack;

			bool expectedOperand = true;

			for (; !isEndOfExpression(*it, allowComma); ++it)
			{
				if (it->isReservedToken())
				{
					OperatorInfo oi = getOperatorInfo(
						it->getReservedToken(), expectedOperand, it->getLineNumber(), it->getCharIndex());

					if (oi.operation == NodeOperation::Call && expectedOperand)
					{
						//open round bracket is misinterpreted as a function call
						++it;
						operandStack.push(parseExpressionTreeImpl(context, it, true, false));
						if (it->hasValue(ReservedToken::CloseRound))
						{
							expectedOperand = false;
							continue;
						}
						else
						{
							throw syntaxError("Expected closing ')'", it->getLineNumber(), it->getCharIndex());
						}
					}

					if (oi.operation == NodeOperation::Init && expectedOperand)
					{
						++it;
						std::vector<NodePtr> children;
						if (!it->hasValue(ReservedToken::CloseCurly))
						{
							while (true)
							{
								children.push_back(parseExpressionTreeImpl(context, it, false, false));
								if (it->hasValue(ReservedToken::CloseCurly))
								{
									break;
								}
								else if (it->hasValue(ReservedToken::Comma))
								{
									++it;
								}
								else
								{
									throw syntaxError("Expected ',', or closing '}'", it->getLineNumber(), it->getCharIndex());
								}
							}
						}
						operandStack.push(std::make_unique<Node>(
							context,
							NodeOperation::Init,
							std::move(children),
							it->getLineNumber(),
							it->getCharIndex()));

						expectedOperand = false;
						continue;
					}

					if ((oi.precedence == OperatorPrecedence::Prefix) != expectedOperand)
					{
						throw unexpectedSyntaxError(
							std::to_string(it->getValue()),
							it->getLineNumber(),
							it->getCharIndex());
					}

					if (!operatorStack.empty() && isEvaluatedBefore(operatorStack.top(), oi))
					{
						popOneOperator(operatorStack, operandStack, context, it->getLineNumber(), it->getCharIndex());
					}

					switch (oi.operation)
					{
					case NodeOperation::Call:
						++it;
						if (!it->hasValue(ReservedToken::CloseRound))
						{
							while (true)
							{
								bool remove_lvalue = !it->hasValue(ReservedToken::BitwiseAnd);
								if (!remove_lvalue)
								{
									++it;
								}
								NodePtr argument = parseExpressionTreeImpl(context, it, false, false);
								if (remove_lvalue)
								{
									size_t lineNumber = argument->getLineNumber();
									size_t charIndex = argument->getCharIndex();
									std::vector<NodePtr> argument_vector;
									argument_vector.push_back(std::move(argument));
									argument = std::make_unique<Node>(
										context,
										NodeOperation::Param,
										std::move(argument_vector),
										lineNumber,
										charIndex);
								}
								else if (!argument->isLvalue())
								{
									throw wrongTypeError(
										std::to_string(argument->getTypeId()),
										std::to_string(argument->getTypeId()),
										true,
										argument->getLineNumber(),
										argument->getCharIndex());
								}

								operandStack.push(std::move(argument));

								++oi.number_of_operands;

								if (it->hasValue(ReservedToken::CloseRound))
								{
									break;
								}
								else if (it->hasValue(ReservedToken::Comma))
								{
									++it;
								}
								else
								{
									throw syntaxError("Expected ',', or closing ')'", it->getLineNumber(), it->getCharIndex());
								}
							}
						}
						break;
					case NodeOperation::Index:
						++it;
						operandStack.push(parseExpressionTreeImpl(context, it, true, false));
						if (!it->hasValue(ReservedToken::CloseSquare))
						{
							throw syntaxError("Expected closing ]'", it->getLineNumber(), it->getCharIndex());
						}
						break;
					case NodeOperation::Ternary:
						++it;
						operandStack.push(parseExpressionTreeImpl(context, it, true, false));
						if (!it->hasValue(ReservedToken::Colon))
						{
							throw syntaxError("Expected ':'", it->getLineNumber(), it->getCharIndex());
						}
						break;
					default:
						break;
					}

					operatorStack.push(oi);

					expectedOperand = (oi.precedence != OperatorPrecedence::Postfix);
				}
				else
				{
					if (!expectedOperand)
					{
						throw unexpectedSyntaxError(
							std::to_string(it->getValue()),
							it->getLineNumber(),
							it->getCharIndex());
					}
					if (it->isNumber())
					{
						operandStack.push(std::make_unique<Node>(
							context, it->getNumber(), std::vector<NodePtr>(), it->getLineNumber(), it->getCharIndex()));
					}
					else if (it->isString())
					{
						operandStack.push(std::make_unique<Node>(
							context, it->getString(), std::vector<NodePtr>(), it->getLineNumber(), it->getCharIndex()));
					}
					else
					{
						operandStack.push(std::make_unique<Node>(
							context, it->getIdentifier(), std::vector<NodePtr>(), it->getLineNumber(), it->getCharIndex()));
					}
					expectedOperand = false;
				}
			}

			if (expectedOperand)
			{
				if (allowEmpty && operandStack.empty() && operatorStack.empty())
				{
					return NodePtr();
				}
				else
				{
					throw syntaxError("Operand expected", it->getLineNumber(), it->getCharIndex());
				}
			}

			while (!operatorStack.empty())
			{
				popOneOperator(operatorStack, operandStack, context, it->getLineNumber(), it->getCharIndex());
			}

			if (operandStack.size() != 1 || !operatorStack.empty())
			{
				throw compilerError("Failed to parse an expression", it->getLineNumber(), it->getCharIndex());
			}

			return std::move(operandStack.top());
		}
	}

	NodePtr parseExpressionTree(
		CompilerContext &context, TokensIterator &it, TypeHandle typeId, bool allowComma)
	{
		NodePtr ret = parseExpressionTreeImpl(context, it, allowComma, typeId == TypeRegistry::getVoidHandle());
		if (ret)
		{
			ret->checkConversion(typeId, false);
		}
		return ret;
	}
}
