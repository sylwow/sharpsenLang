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
			brackets,
			postfix,
			prefix,
			multiplication,
			addition,
			shift,
			comparison,
			equality,
			bitwise_and,
			bitwise_xor,
			bitwise_or,
			logical_and,
			logical_or,
			assignment,
			comma
		};

		enum struct OperatorAssociativity
		{
			left_to_right,
			right_to_left
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
				case NodeOperation::init:
					precedence = OperatorPrecedence::brackets;
					break;
				case NodeOperation::param: // This will never happen. Used only for the Node creation.
				case NodeOperation::postinc:
				case NodeOperation::postdec:
				case NodeOperation::index:
				case NodeOperation::call:
					precedence = OperatorPrecedence::postfix;
					break;
				case NodeOperation::preinc:
				case NodeOperation::predec:
				case NodeOperation::positive:
				case NodeOperation::negative:
				case NodeOperation::bnot:
				case NodeOperation::lnot:
				case NodeOperation::size:
				case NodeOperation::tostring:
					precedence = OperatorPrecedence::prefix;
					break;
				case NodeOperation::mul:
				case NodeOperation::div:
				case NodeOperation::idiv:
				case NodeOperation::mod:
					precedence = OperatorPrecedence::multiplication;
					break;
				case NodeOperation::add:
				case NodeOperation::sub:
				case NodeOperation::concat:
					precedence = OperatorPrecedence::addition;
					break;
				case NodeOperation::bsl:
				case NodeOperation::bsr:
					precedence = OperatorPrecedence::shift;
					break;
				case NodeOperation::lt:
				case NodeOperation::gt:
				case NodeOperation::le:
				case NodeOperation::ge:
					precedence = OperatorPrecedence::comparison;
					break;
				case NodeOperation::eq:
				case NodeOperation::ne:
					precedence = OperatorPrecedence::equality;
					break;
				case NodeOperation::band:
					precedence = OperatorPrecedence::bitwise_and;
					break;
				case NodeOperation::bxor:
					precedence = OperatorPrecedence::bitwise_xor;
					break;
				case NodeOperation::bor:
					precedence = OperatorPrecedence::bitwise_or;
					break;
				case NodeOperation::land:
					precedence = OperatorPrecedence::logical_and;
					break;
				case NodeOperation::lor:
					precedence = OperatorPrecedence::logical_or;
					break;
				case NodeOperation::assign:
				case NodeOperation::add_assign:
				case NodeOperation::sub_assign:
				case NodeOperation::mul_assign:
				case NodeOperation::div_assign:
				case NodeOperation::idiv_assign:
				case NodeOperation::mod_assign:
				case NodeOperation::band_assign:
				case NodeOperation::bor_assign:
				case NodeOperation::bxor_assign:
				case NodeOperation::bsl_assign:
				case NodeOperation::bsr_assign:
				case NodeOperation::concat_assign:
				case NodeOperation::ternary:
					precedence = OperatorPrecedence::assignment;
					break;
				case NodeOperation::comma:
					precedence = OperatorPrecedence::comma;
					break;
				}

				switch (precedence)
				{
				case OperatorPrecedence::prefix:
				case OperatorPrecedence::assignment:
					associativity = OperatorAssociativity::right_to_left;
					break;
				default:
					associativity = OperatorAssociativity::left_to_right;
					break;
				}

				switch (operation)
				{
				case NodeOperation::init:
					number_of_operands = 0; //zero or more
					break;
				case NodeOperation::postinc:
				case NodeOperation::postdec:
				case NodeOperation::preinc:
				case NodeOperation::predec:
				case NodeOperation::positive:
				case NodeOperation::negative:
				case NodeOperation::bnot:
				case NodeOperation::lnot:
				case NodeOperation::size:
				case NodeOperation::tostring:
				case NodeOperation::call: //at least one
					number_of_operands = 1;
					break;
				case NodeOperation::ternary:
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
			case ReservedToken::inc:
				return prefix ? OperatorInfo(NodeOperation::preinc, lineNumber, charIndex)
							  : OperatorInfo(NodeOperation::postinc, lineNumber, charIndex);
			case ReservedToken::dec:
				return prefix ? OperatorInfo(NodeOperation::predec, lineNumber, charIndex)
							  : OperatorInfo(NodeOperation::postdec, lineNumber, charIndex);
			case ReservedToken::add:
				return prefix ? OperatorInfo(NodeOperation::positive, lineNumber, charIndex)
							  : OperatorInfo(NodeOperation::add, lineNumber, charIndex);
			case ReservedToken::sub:
				return prefix ? OperatorInfo(NodeOperation::negative, lineNumber, charIndex)
							  : OperatorInfo(NodeOperation::sub, lineNumber, charIndex);
			case ReservedToken::concat:
				return OperatorInfo(NodeOperation::concat, lineNumber, charIndex);
			case ReservedToken::mul:
				return OperatorInfo(NodeOperation::mul, lineNumber, charIndex);
			case ReservedToken::div:
				return OperatorInfo(NodeOperation::div, lineNumber, charIndex);
			case ReservedToken::idiv:
				return OperatorInfo(NodeOperation::idiv, lineNumber, charIndex);
			case ReservedToken::mod:
				return OperatorInfo(NodeOperation::mod, lineNumber, charIndex);
			case ReservedToken::bitwise_not:
				return OperatorInfo(NodeOperation::bnot, lineNumber, charIndex);
			case ReservedToken::bitwise_and:
				return OperatorInfo(NodeOperation::band, lineNumber, charIndex);
			case ReservedToken::bitwise_or:
				return OperatorInfo(NodeOperation::bor, lineNumber, charIndex);
			case ReservedToken::bitwise_xor:
				return OperatorInfo(NodeOperation::bxor, lineNumber, charIndex);
			case ReservedToken::shiftl:
				return OperatorInfo(NodeOperation::bsl, lineNumber, charIndex);
			case ReservedToken::shiftr:
				return OperatorInfo(NodeOperation::bsr, lineNumber, charIndex);
			case ReservedToken::assign:
				return OperatorInfo(NodeOperation::assign, lineNumber, charIndex);
			case ReservedToken::add_assign:
				return OperatorInfo(NodeOperation::add_assign, lineNumber, charIndex);
			case ReservedToken::sub_assign:
				return OperatorInfo(NodeOperation::sub_assign, lineNumber, charIndex);
			case ReservedToken::concat_assign:
				return OperatorInfo(NodeOperation::concat_assign, lineNumber, charIndex);
			case ReservedToken::mul_assign:
				return OperatorInfo(NodeOperation::mod_assign, lineNumber, charIndex);
			case ReservedToken::div_assign:
				return OperatorInfo(NodeOperation::div_assign, lineNumber, charIndex);
			case ReservedToken::idiv_assign:
				return OperatorInfo(NodeOperation::idiv_assign, lineNumber, charIndex);
			case ReservedToken::mod_assign:
				return OperatorInfo(NodeOperation::mod_assign, lineNumber, charIndex);
			case ReservedToken::and_assign:
				return OperatorInfo(NodeOperation::band_assign, lineNumber, charIndex);
			case ReservedToken::or_assign:
				return OperatorInfo(NodeOperation::bor_assign, lineNumber, charIndex);
			case ReservedToken::xor_assign:
				return OperatorInfo(NodeOperation::bxor_assign, lineNumber, charIndex);
			case ReservedToken::shiftl_assign:
				return OperatorInfo(NodeOperation::bsl_assign, lineNumber, charIndex);
			case ReservedToken::shiftr_assign:
				return OperatorInfo(NodeOperation::bsr_assign, lineNumber, charIndex);
			case ReservedToken::logical_not:
				return OperatorInfo(NodeOperation::lnot, lineNumber, charIndex);
			case ReservedToken::logical_and:
				return OperatorInfo(NodeOperation::land, lineNumber, charIndex);
			case ReservedToken::logical_or:
				return OperatorInfo(NodeOperation::lor, lineNumber, charIndex);
			case ReservedToken::eq:
				return OperatorInfo(NodeOperation::eq, lineNumber, charIndex);
			case ReservedToken::ne:
				return OperatorInfo(NodeOperation::ne, lineNumber, charIndex);
			case ReservedToken::lt:
				return OperatorInfo(NodeOperation::lt, lineNumber, charIndex);
			case ReservedToken::gt:
				return OperatorInfo(NodeOperation::gt, lineNumber, charIndex);
			case ReservedToken::le:
				return OperatorInfo(NodeOperation::le, lineNumber, charIndex);
			case ReservedToken::ge:
				return OperatorInfo(NodeOperation::ge, lineNumber, charIndex);
			case ReservedToken::question:
				return OperatorInfo(NodeOperation::ternary, lineNumber, charIndex);
			case ReservedToken::comma:
				return OperatorInfo(NodeOperation::comma, lineNumber, charIndex);
			case ReservedToken::open_round:
				return OperatorInfo(NodeOperation::call, lineNumber, charIndex);
			case ReservedToken::open_square:
				return OperatorInfo(NodeOperation::index, lineNumber, charIndex);
			case ReservedToken::kw_sizeof:
				return OperatorInfo(NodeOperation::size, lineNumber, charIndex);
			case ReservedToken::kw_tostring:
				return OperatorInfo(NodeOperation::tostring, lineNumber, charIndex);
			case ReservedToken::open_curly:
				return OperatorInfo(NodeOperation::init, lineNumber, charIndex);
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
				case ReservedToken::semicolon:
				case ReservedToken::close_round:
				case ReservedToken::close_square:
				case ReservedToken::close_curly:
				case ReservedToken::colon:
					return true;
				case ReservedToken::comma:
					return !allowComma;
				default:
					return false;
				}
			}

			return false;
		}

		bool isEvaluatedBefore(const OperatorInfo &l, const OperatorInfo &r)
		{
			return l.associativity == OperatorAssociativity::left_to_right ? l.precedence <= r.precedence : l.precedence < r.precedence;
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

			if (operatorStack.top().precedence != OperatorPrecedence::prefix)
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

					if (oi.operation == NodeOperation::call && expectedOperand)
					{
						//open round bracket is misinterpreted as a function call
						++it;
						operandStack.push(parseExpressionTreeImpl(context, it, true, false));
						if (it->hasValue(ReservedToken::close_round))
						{
							expectedOperand = false;
							continue;
						}
						else
						{
							throw syntaxError("Expected closing ')'", it->getLineNumber(), it->getCharIndex());
						}
					}

					if (oi.operation == NodeOperation::init && expectedOperand)
					{
						++it;
						std::vector<NodePtr> children;
						if (!it->hasValue(ReservedToken::close_curly))
						{
							while (true)
							{
								children.push_back(parseExpressionTreeImpl(context, it, false, false));
								if (it->hasValue(ReservedToken::close_curly))
								{
									break;
								}
								else if (it->hasValue(ReservedToken::comma))
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
							NodeOperation::init,
							std::move(children),
							it->getLineNumber(),
							it->getCharIndex()));

						expectedOperand = false;
						continue;
					}

					if ((oi.precedence == OperatorPrecedence::prefix) != expectedOperand)
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
					case NodeOperation::call:
						++it;
						if (!it->hasValue(ReservedToken::close_round))
						{
							while (true)
							{
								bool remove_lvalue = !it->hasValue(ReservedToken::bitwise_and);
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
										NodeOperation::param,
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

								if (it->hasValue(ReservedToken::close_round))
								{
									break;
								}
								else if (it->hasValue(ReservedToken::comma))
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
					case NodeOperation::index:
						++it;
						operandStack.push(parseExpressionTreeImpl(context, it, true, false));
						if (!it->hasValue(ReservedToken::close_square))
						{
							throw syntaxError("Expected closing ]'", it->getLineNumber(), it->getCharIndex());
						}
						break;
					case NodeOperation::ternary:
						++it;
						operandStack.push(parseExpressionTreeImpl(context, it, true, false));
						if (!it->hasValue(ReservedToken::colon))
						{
							throw syntaxError("Expected ':'", it->getLineNumber(), it->getCharIndex());
						}
						break;
					default:
						break;
					}

					operatorStack.push(oi);

					expectedOperand = (oi.precedence != OperatorPrecedence::postfix);
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
		CompilerContext &context, TokensIterator &it, TypeHandle type_id, bool allowComma)
	{
		NodePtr ret = parseExpressionTreeImpl(context, it, allowComma, type_id == TypeRegistry::getVoidHandle());
		if (ret)
		{
			ret->checkConversion(type_id, false);
		}
		return ret;
	}
}
