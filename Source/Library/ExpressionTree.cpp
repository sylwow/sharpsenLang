#include "ExpressionTree.hpp"
#include "Helpers.hpp"
#include "Errors.hpp"
#include "CompilerContext.hpp"

namespace sharpsenLang
{
	namespace
	{
		bool isConvertible(TypeHandle type_from, bool lvalue_from, TypeHandle type_to, bool lvalue_to)
		{
			if (type_to == TypeRegistry::getVoidHandle())
			{
				return true;
			}
			if (lvalue_to)
			{
				return lvalue_from && type_from == type_to;
			}
			if (type_from == type_to)
			{
				return true;
			}
			if (const InitListType *ilt = std::get_if<InitListType>(type_from))
			{
				if (lvalue_to)
				{
					return false;
				}
				if (type_to == TypeRegistry::getVoidHandle())
				{
					return true;
				}
				return std::visit(overloaded{[&](const ArrayType &at)
											 {
												 for (TypeHandle it : ilt->innerTypeId)
												 {
													 if (it != at.innerTypeId)
													 {
														 return false;
													 }
												 }
												 return true;
											 },
											 [&](const TupleType &tt)
											 {
												 if (tt.innerTypeId.size() != ilt->innerTypeId.size())
												 {
													 return false;
												 }
												 for (size_t i = 0; i < tt.innerTypeId.size(); ++i)
												 {
													 if (ilt->innerTypeId[i] != tt.innerTypeId[i])
													 {
														 return false;
													 }
												 }
												 return true;
											 },
											 [&](const Type &)
											 {
												 return false;
											 }},
								  *type_to);
			}
			return type_from == TypeRegistry::getNumberHandle() && type_to == TypeRegistry::getStringHandle();
		}
	}

	Node::Node(CompilerContext &context, NodeValue value, std::vector<NodePtr> children, size_t line_number, size_t char_index)
		: _value(std::move(value)),
		  _children(std::move(children)),
		  _lineNumber(line_number),
		  _charIndex(char_index)
	{
		const TypeHandle void_handle = TypeRegistry::getVoidHandle();
		const TypeHandle number_handle = TypeRegistry::getNumberHandle();
		const TypeHandle string_handle = TypeRegistry::getStringHandle();

		std::visit(
			overloaded{
				[&](const std::string &value)
				{
					_typeId = string_handle;
					_lvalue = false;
				},
				[&](double value)
				{
					_typeId = number_handle;
					_lvalue = false;
				},
				[&](const Identifier &value)
				{
					if (const IdentifierInfo *info = context.find(value.name))
					{
						_typeId = info->typeId();
						_lvalue = (info->getScope() != IdentifierScope::Function);
					}
					else
					{
						throw undeclaredError(value.name, _lineNumber, _charIndex);
					}
				},
				[&](NodeOperation value)
				{
					switch (value)
					{
					case NodeOperation::Param:
						_typeId = _children[0]->_typeId;
						_lvalue = false;
						break;
					case NodeOperation::Preinc:
					case NodeOperation::Predec:
						_typeId = number_handle;
						_lvalue = true;
						_children[0]->checkConversion(number_handle, true);
						break;
					case NodeOperation::Postinc:
					case NodeOperation::Postdec:
						_typeId = number_handle;
						_lvalue = false;
						_children[0]->checkConversion(number_handle, true);
						break;
					case NodeOperation::Positive:
					case NodeOperation::Negative:
					case NodeOperation::Bnot:
					case NodeOperation::Lnot:
						_typeId = number_handle;
						_lvalue = false;
						_children[0]->checkConversion(number_handle, false);
						break;
					case NodeOperation::Size:
						_typeId = number_handle;
						_lvalue = false;
						break;
					case NodeOperation::ToString:
						_typeId = string_handle;
						_lvalue = false;
						break;
					case NodeOperation::Add:
					case NodeOperation::Sub:
					case NodeOperation::Mul:
					case NodeOperation::Div:
					case NodeOperation::Idiv:
					case NodeOperation::Mod:
					case NodeOperation::Band:
					case NodeOperation::Bor:
					case NodeOperation::Bxor:
					case NodeOperation::Bsl:
					case NodeOperation::Bsr:
					case NodeOperation::Land:
					case NodeOperation::Lor:
						_typeId = number_handle;
						_lvalue = false;
						_children[0]->checkConversion(number_handle, false);
						_children[1]->checkConversion(number_handle, false);
						break;
					case NodeOperation::Eq:
					case NodeOperation::Ne:
					case NodeOperation::Lt:
					case NodeOperation::Gt:
					case NodeOperation::Le:
					case NodeOperation::Ge:
						_typeId = number_handle;
						_lvalue = false;
						if (!_children[0]->isNumber() || !_children[1]->isNumber())
						{
							_children[0]->checkConversion(string_handle, false);
							_children[1]->checkConversion(string_handle, false);
						}
						else
						{
							_children[0]->checkConversion(number_handle, false);
							_children[1]->checkConversion(number_handle, false);
						}
						break;
					case NodeOperation::Concat:
						_typeId = context.getHandle(SimpleType::String);
						_lvalue = false;
						_children[0]->checkConversion(string_handle, false);
						_children[1]->checkConversion(string_handle, false);
						break;
					case NodeOperation::Assign:
						_typeId = _children[0]->getTypeId();
						_lvalue = true;
						_children[0]->checkConversion(_typeId, true);
						_children[1]->checkConversion(_typeId, false);
						break;
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
						_typeId = number_handle;
						_lvalue = true;
						_children[0]->checkConversion(number_handle, true);
						_children[1]->checkConversion(number_handle, false);
						break;
					case NodeOperation::ConcatAssign:
						_typeId = string_handle;
						_lvalue = true;
						_children[0]->checkConversion(string_handle, true);
						_children[1]->checkConversion(string_handle, false);
						break;
					case NodeOperation::Comma:
						for (int i = 0; i < int(_children.size()) - 1; ++i)
						{
							_children[i]->checkConversion(void_handle, false);
						}
						_typeId = _children.back()->getTypeId();
						_lvalue = _children.back()->isLvalue();
						break;
					case NodeOperation::Index:
						_lvalue = _children[0]->isLvalue();
						if (const ArrayType *at = std::get_if<ArrayType>(_children[0]->getTypeId()))
						{
							_typeId = at->innerTypeId;
						}
						else if (const TupleType *tt = std::get_if<TupleType>(_children[0]->getTypeId()))
						{
							if (_children[1]->isNumber())
							{
								double idx = _children[1]->getNumber();
								if (size_t(idx) == idx && idx >= 0 && idx < tt->innerTypeId.size())
								{
									_typeId = tt->innerTypeId[size_t(idx)];
								}
								else
								{
									throw semanticError("Invalid tuple index " + std::to_string(idx), _lineNumber, _charIndex);
								}
							}
							else
							{
								throw semanticError("Invalid tuple index", _lineNumber, _charIndex);
							}
						}
						else
						{
							throw semanticError(to_string(_children[0]->_typeId) + " is not indexable",
												_lineNumber, _charIndex);
						}
						break;
					case NodeOperation::Ternary:
						_children[0]->checkConversion(number_handle, false);
						if (isConvertible(
								_children[2]->getTypeId(), _children[2]->isLvalue(),
								_children[1]->getTypeId(), _children[1]->isLvalue()))
						{
							_children[2]->checkConversion(_children[1]->getTypeId(), _children[1]->isLvalue());
							_typeId = _children[1]->getTypeId();
							_lvalue = _children[1]->isLvalue();
						}
						else
						{
							_children[1]->checkConversion(_children[2]->getTypeId(), _children[2]->isLvalue());
							_typeId = _children[2]->getTypeId();
							_lvalue = _children[2]->isLvalue();
						}
						break;
					case NodeOperation::Call:
						if (const FunctionType *ft = std::get_if<FunctionType>(_children[0]->getTypeId()))
						{
							_typeId = ft->returnTypeId;
							_lvalue = false;
							if (ft->paramTypeId.size() + 1 != _children.size())
							{
								throw semanticError("Wrong number of arguments. "
													"Expected " +
														std::to_string(ft->paramTypeId.size()) +
														", given " + std::to_string(_children.size() - 1),
													_lineNumber, _charIndex);
							}
							for (size_t i = 0; i < ft->paramTypeId.size(); ++i)
							{
								if (_children[i + 1]->isLvalue() && !ft->paramTypeId[i].byRef)
								{
									throw semanticError(
										"Function doesn't receive the argument by reference",
										_children[i + 1]->getLineNumber(), _children[i + 1]->getCharIndex());
								}
								_children[i + 1]->checkConversion(ft->paramTypeId[i].typeId, ft->paramTypeId[i].byRef);
							}
						}
						else
						{
							throw semanticError(to_string(_children[0]->_typeId) + " is not callable",
												_lineNumber, _charIndex);
						}
						break;
					case NodeOperation::Init:
					{
						InitListType ilt;
						ilt.innerTypeId.reserve(_children.size());
						for (const NodePtr &child : _children)
						{
							ilt.innerTypeId.push_back(child->getTypeId());
						}
						_typeId = context.getHandle(ilt);
						_lvalue = false;
						break;
					}
					}
				}},
			_value);
	}

	const NodeValue &Node::getValue() const
	{
		return _value;
	}

	bool Node::isNodeOperation() const
	{
		return std::holds_alternative<NodeOperation>(_value);
	}

	bool Node::isIdentifier() const
	{
		return std::holds_alternative<Identifier>(_value);
	}

	bool Node::isNumber() const
	{
		return std::holds_alternative<double>(_value);
	}

	bool Node::isString() const
	{
		return std::holds_alternative<std::string>(_value);
	}

	NodeOperation Node::getNodeOperation() const
	{
		return std::get<NodeOperation>(_value);
	}

	std::string_view Node::getIdentifier() const
	{
		return std::get<Identifier>(_value).name;
	}

	double Node::getNumber() const
	{
		return std::get<double>(_value);
	}

	std::string_view Node::getString() const
	{
		return std::get<std::string>(_value);
	}

	const std::vector<NodePtr> &Node::getChildren() const
	{
		return _children;
	}

	TypeHandle Node::getTypeId() const
	{
		return _typeId;
	}

	bool Node::isLvalue() const
	{
		return _lvalue;
	}

	size_t Node::getLineNumber() const
	{
		return _lineNumber;
	}

	size_t Node::getCharIndex() const
	{
		return _charIndex;
	}

	void Node::checkConversion(TypeHandle typeId, bool lvalue) const
	{
		if (!isConvertible(_typeId, _lvalue, typeId, lvalue))
		{
			throw wrongTypeError(std::to_string(_typeId), std::to_string(typeId), lvalue,
								 _lineNumber, _charIndex);
		}
	}
}
