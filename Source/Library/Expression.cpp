#include <type_traits>

#include "Expression.hpp"
#include "ExpressionTree.hpp"
#include "ExpressionTreeParser.hpp"
#include "Helpers.hpp"
#include "Errors.hpp"
#include "RuntimeContext.hpp"
#include "Tokenizer.hpp"
#include "CompilerContext.hpp"

namespace sharpsenLang
{
	namespace
	{
		template <class V, typename T>
		struct IsBoxed
		{
			static const bool value = false;
		};

		template <typename T>
		struct IsBoxed<std::shared_ptr<VariableImpl<T>>, T>
		{
			static const bool value = true;
		};

		template <typename T>
		struct RemoveCvref
		{
			using type = typename std::remove_cv<typename std::remove_reference<T>::type>::type;
		};

		template <typename T>
		auto unbox(T &&t)
		{
			if constexpr (std::is_same<typename RemoveCvref<T>::type, Larray>::value)
			{
				return cloneVariableValue(t->value);
			}
			else
			{
				return t->value;
			}
		}

		template <typename To, typename From>
		auto convert(From &&from)
		{
			if constexpr (std::is_convertible<From, To>::value)
			{
				return std::forward<From>(from);
			}
			else if constexpr (IsBoxed<From, To>::value)
			{
				return unbox(std::forward<From>(from));
			}
			else if constexpr (std::is_same<To, String>::value)
			{
				return convertToString(from);
			}
			else
			{
				static_assert(std::is_void<To>::value);
			}
		}

		template <typename From, typename To>
		struct IsConvertible
		{
			static const bool value =
				std::is_convertible<From, To>::value ||
				IsBoxed<From, To>::value ||
				(std::is_same<To, String>::value &&
				 (std::is_same<From, Number>::value ||
				  std::is_same<From, Lnumber>::value)) ||
				std::is_void<To>::value;
		};

		Number lt(Number n1, Number n2)
		{
			return n1 < n2;
		}

		Number lt(String s1, String s2)
		{
			return *s1 < *s2;
		}

		template <typename R, typename T>
		class GlobalVariableExpression : public Expression<R>
		{
		private:
			int _idx;

		public:
			GlobalVariableExpression(int idx)
				: _idx(idx)
			{
			}

			R evaluate(RuntimeContext &context) const override
			{
				return convert<R>(context.global(_idx)->template staticPointerDowncast<T>());
			}
		};

		template <typename R, typename T>
		class LocalVariableExpression : public Expression<R>
		{
		private:
			int _idx;

		public:
			LocalVariableExpression(int idx)
				: _idx(idx)
			{
			}

			R evaluate(RuntimeContext &context) const override
			{
				return convert<R>(context.local(_idx)->template staticPointerDowncast<T>());
			}
		};

		template <typename R>
		class FunctionExpression : public Expression<R>
		{
		private:
			int _idx;

		public:
			FunctionExpression(int idx)
				: _idx(idx)
			{
			}

			R evaluate(RuntimeContext &context) const override
			{
				return convert<R>(context.getFunction(_idx));
			}
		};

		template <typename R, typename T>
		class ConstantExpression : public Expression<R>
		{
		private:
			T _c;

		public:
			ConstantExpression(T c)
				: _c(std::move(c))
			{
			}

			R evaluate(RuntimeContext &context) const override
			{
				return convert<R>(_c);
			}
		};

		template <class O, typename R, typename... Ts>
		class GenericExpression : public Expression<R>
		{
		private:
			std::tuple<typename Expression<Ts>::Ptr...> _exprs;

			template <typename... Exprs>
			R evaluateTuple(RuntimeContext &context, const Exprs &...exprs) const
			{
				if constexpr (std::is_same<R, void>::value)
				{
					O()
					(std::move(exprs->evaluate(context))...);
				}
				else
				{
					return convert<R>(O()(std::move(exprs->evaluate(context))...));
				}
			}

		public:
			GenericExpression(typename Expression<Ts>::Ptr... exprs)
				: _exprs(std::move(exprs)...)
			{
			}

			R evaluate(RuntimeContext &context) const override
			{
				return std::apply(
					[&](const auto &...exprs)
					{
						return this->evaluateTuple(context, exprs...);
					},
					_exprs);
			}
		};

#define UNARY_EXPRESSION(name, code)   \
	struct name##Op                    \
	{                                  \
		template <typename T1>         \
		auto operator()(T1 t1)         \
		{                              \
			code;                      \
		}                              \
	};                                 \
	template <typename R, typename T1> \
	using name##Expression = GenericExpression<name##Op, R, T1>;

		UNARY_EXPRESSION(Preinc,
						 ++t1->value;
						 return t1;);

		UNARY_EXPRESSION(Predec,
						 --t1->value;
						 return t1;);

		UNARY_EXPRESSION(Postinc, return t1->value++);

		UNARY_EXPRESSION(Postdec, return t1->value--);

		UNARY_EXPRESSION(Positive, return t1);

		UNARY_EXPRESSION(Negative, return -t1);

		UNARY_EXPRESSION(Bnot, return ~int(t1));

		UNARY_EXPRESSION(Lnot, return !t1);

		UNARY_EXPRESSION(Size,
						 return t1->value.size(););

		UNARY_EXPRESSION(ToString,
						 return convertToString(t1););

#undef UNARY_EXPRESSION

#define BINARY_EXPRESSION(name, code)               \
	struct name##Op                                 \
	{                                               \
		template <typename T1, typename T2>         \
		auto operator()(T1 t1, T2 t2)               \
		{                                           \
			code;                                   \
		}                                           \
	};                                              \
	template <typename R, typename T1, typename T2> \
	using name##Expression = GenericExpression<name##Op, R, T1, T2>;

		BINARY_EXPRESSION(Add, return t1 + t2);

		BINARY_EXPRESSION(Sub, return t1 - t2);

		BINARY_EXPRESSION(Mul, return t1 * t2);

		BINARY_EXPRESSION(Div, return t1 / t2);

		BINARY_EXPRESSION(Idiv, return int(t1 / t2));

		BINARY_EXPRESSION(Mod, return t1 - t2 * int(t1 / t2));

		BINARY_EXPRESSION(Band, return int(t1) & int(t2));

		BINARY_EXPRESSION(Bor, return int(t1) | int(t2));

		BINARY_EXPRESSION(Bxor, return int(t1) ^ int(t2));

		BINARY_EXPRESSION(Bsl, return int(t1) << int(t2));

		BINARY_EXPRESSION(Bsr, return int(t1) >> int(t2));

		BINARY_EXPRESSION(Concat, return std::make_shared<std::string>(*t1 + *t2));

		BINARY_EXPRESSION(AddAssign,
						  t1->value += t2;
						  return t1;);

		BINARY_EXPRESSION(SubAssign,
						  t1->value -= t2;
						  return t1;);

		BINARY_EXPRESSION(MulAssign,
						  t1->value *= t2;
						  return t1;);

		BINARY_EXPRESSION(DivAssign,
						  t1->value /= t2;
						  return t1;);

		BINARY_EXPRESSION(IdivAssign,
						  t1->value = int(t1->value / t2);
						  return t1;);

		BINARY_EXPRESSION(ModAssign,
						  t1->value = t1->value - t2 * int(t1->value / t2);
						  ;
						  return t1;);

		BINARY_EXPRESSION(BandAssign,
						  t1->value = int(t1->value) & int(t2);
						  return t1;);

		BINARY_EXPRESSION(BorAssign,
						  t1->value = int(t1->value) | int(t2);
						  return t1;);

		BINARY_EXPRESSION(BxorAssign,
						  t1->value = int(t1->value) ^ int(t2);
						  return t1;);

		BINARY_EXPRESSION(BslAssign,
						  t1->value = int(t1->value) << int(t2);
						  return t1;);

		BINARY_EXPRESSION(BsrAssign,
						  t1->value = int(t1->value) >> int(t2);
						  return t1;);

		BINARY_EXPRESSION(ConcatAssign,
						  t1->value = std::make_shared<std::string>(*t1->value + *t2);
						  return t1;);

		BINARY_EXPRESSION(Assign,
						  t1->value = std::move(t2);
						  return t1;);

		BINARY_EXPRESSION(Eq, return !lt(t1, t2) && !lt(t2, t1));

		BINARY_EXPRESSION(Ne, return lt(t1, t2) || lt(t2, t1));

		BINARY_EXPRESSION(Lt, return lt(t1, t2));

		BINARY_EXPRESSION(Gt, return lt(t2, t1));

		BINARY_EXPRESSION(Le, return !lt(t2, t1));

		BINARY_EXPRESSION(Ge, return !lt(t1, t2));

#undef BINARY_EXPRESSION

		template <typename R, typename T1, typename T2>
		class CommaExpression : public Expression<R>
		{
		private:
			typename Expression<T1>::Ptr _expr1;
			typename Expression<T2>::Ptr _expr2;

		public:
			CommaExpression(typename Expression<T1>::Ptr expr1, typename Expression<T2>::Ptr expr2)
				: _expr1(std::move(expr1)),
				  _expr2(std::move(expr2))
			{
			}

			R evaluate(RuntimeContext &context) const override
			{
				_expr1->evaluate(context);

				if constexpr (std::is_same<R, void>::value)
				{
					_expr2->evaluate(context);
				}
				else
				{
					return convert<R>(_expr2->evaluate(context));
				}
			}
		};

		template <typename R, typename T1, typename T2>
		class LandExpression : public Expression<R>
		{
		private:
			typename Expression<T1>::Ptr _expr1;
			typename Expression<T2>::Ptr _expr2;

		public:
			LandExpression(typename Expression<T1>::Ptr expr1, typename Expression<T2>::Ptr expr2)
				: _expr1(std::move(expr1)),
				  _expr2(std::move(expr2))
			{
			}

			R evaluate(RuntimeContext &context) const override
			{
				return convert<R>(_expr1->evaluate(context) && _expr2->evaluate(context));
			}
		};

		template <typename R, typename T1, typename T2>
		class LorExpression : public Expression<R>
		{
		private:
			typename Expression<T1>::Ptr _expr1;
			typename Expression<T2>::Ptr _expr2;

		public:
			LorExpression(typename Expression<T1>::Ptr expr1, typename Expression<T2>::Ptr expr2)
				: _expr1(std::move(expr1)),
				  _expr2(std::move(expr2))
			{
			}

			R evaluate(RuntimeContext &context) const override
			{
				return convert<R>(_expr1->evaluate(context) || _expr2->evaluate(context));
			}
		};

		template <typename R, typename T1, typename T2, typename T3>
		class TernaryExpression : public Expression<R>
		{
		private:
			typename Expression<T1>::Ptr _expr1;
			typename Expression<T2>::Ptr _expr2;
			typename Expression<T3>::Ptr _expr3;

		public:
			TernaryExpression(
				typename Expression<T1>::Ptr expr1,
				typename Expression<T2>::Ptr expr2,
				typename Expression<T2>::Ptr expr3)
				: _expr1(std::move(expr1)),
				  _expr2(std::move(expr2)),
				  _expr3(std::move(expr3))
			{
			}

			R evaluate(RuntimeContext &context) const override
			{
				if constexpr (std::is_same<R, void>::value)
				{
					_expr1->evaluate(context) ? _expr2->evaluate(context) : _expr3->evaluate(context);
				}
				else
				{
					return convert<R>(
						_expr1->evaluate(context) ? _expr2->evaluate(context) : _expr3->evaluate(context));
				}
			}
		};

		template <typename R, typename A, typename T>
		class IndexExpression : public Expression<R>
		{
		private:
			typename Expression<A>::Ptr _expr1;
			Expression<Number>::Ptr _expr2;
			Expression<Lvalue>::Ptr _init;

			static Array &value(A &arr)
			{
				if constexpr (std::is_same<Larray, A>::value)
				{
					return arr->value;
				}
				else
				{
					static_assert(std::is_same<Array, A>::value);
					return arr;
				}
			}

			static auto toLvalueImpl(Lvalue v)
			{
				if constexpr (std::is_same<Larray, A>::value)
				{
					return v->staticPointerDowncast<T>();
				}
				else
				{
					static_assert(std::is_same<Array, A>::value);
					return std::static_pointer_cast<VariableImpl<T>>(v);
				}
			}

		public:
			IndexExpression(typename Expression<A>::Ptr expr1, Expression<Number>::Ptr expr2, Expression<Lvalue>::Ptr init)
				: _expr1(std::move(expr1)),
				  _expr2(std::move(expr2)),
				  _init(std::move(init))
			{
			}

			R evaluate(RuntimeContext &context) const override
			{
				A arr = _expr1->evaluate(context);
				int idx = int(_expr2->evaluate(context));

				runtimeAssertion(idx >= 0, "Negative index is invalid");

				while (idx >= value(arr).size())
				{
					value(arr).push_back(_init->evaluate(context));
				}

				return convert<R>(
					toLvalueImpl(value(arr)[idx]));
			}
		};

		template <typename R, typename A, typename T>
		class MemberExpression : public Expression<R>
		{
		private:
			typename Expression<A>::Ptr _expr;
			size_t _idx;

			static Tuple &value(A &arr)
			{
				if constexpr (std::is_same<Ltuple, A>::value)
				{
					return arr->value;
				}
				else
				{
					static_assert(std::is_same<Tuple, A>::value);
					return arr;
				}
			}

			static auto toLvalueImpl(Lvalue v)
			{
				if constexpr (std::is_same<Larray, A>::value)
				{
					return v->staticPointerDowncast<T>();
				}
				else
				{
					static_assert(std::is_same<Array, A>::value);
					return std::static_pointer_cast<VariableImpl<T>>(v);
				}
			}

		public:
			MemberExpression(typename Expression<A>::Ptr expr, size_t idx)
				: _expr(std::move(expr)),
				  _idx(idx)
			{
			}

			R evaluate(RuntimeContext &context) const override
			{
				A tup = _expr->evaluate(context);

				return convert<R>(
					toLvalueImpl(value(tup)[_idx]));
			}
		};

		template <typename R, typename A, typename T>
		class ClassMemberExpression : public Expression<R>
		{
		private:
			typename Expression<A>::Ptr _expr;
			size_t _idx;

			static Class &value(A &arr)
			{
				if constexpr (std::is_same<Lclass, A>::value)
				{
					return arr->value;
				}
				else
				{
					static_assert(std::is_same<Class, A>::value);
					return arr;
				}
			}

			static auto toLvalueImpl(Lvalue v)
			{
				if constexpr (std::is_same<Lclass, A>::value)
				{
					return v->staticPointerDowncast<T>();
				}
				else
				{
					static_assert(std::is_same<Class, A>::value);
					return std::static_pointer_cast<VariableImpl<T>>(v);
				}
			}

		public:
			ClassMemberExpression(typename Expression<A>::Ptr expr, size_t idx)
				: _expr(std::move(expr)),
				  _idx(idx)
			{
			}

			R evaluate(RuntimeContext &context) const override
			{
				A tup = _expr->evaluate(context);

				return convert<R>(
					toLvalueImpl(value(tup).properties[_idx]));
			}
		};

		template <typename R, typename T>
		class CallExpression : public Expression<R>
		{
		private:
			Expression<Function>::Ptr _fexpr;
			std::vector<Expression<Lvalue>::Ptr> _exprs;

		public:
			CallExpression(
				Expression<Function>::Ptr fexpr,
				std::vector<Expression<Lvalue>::Ptr> exprs)
				: _fexpr(std::move(fexpr)),
				  _exprs(std::move(exprs))
			{
			}

			R evaluate(RuntimeContext &context) const override
			{
				std::vector<VariablePtr> params;
				params.reserve(_exprs.size());

				for (size_t i = 0; i < _exprs.size(); ++i)
				{
					params.push_back(_exprs[i]->evaluate(context));
				}

				Function f = _fexpr->evaluate(context);

				if constexpr (std::is_same<R, void>::value)
				{
					context.call(f, std::move(params));
				}
				else
				{
					return convert<R>(std::move(
						std::static_pointer_cast<VariableImpl<T>>(context.call(f, std::move(params)))->value));
				}
			}
		};

		template <typename R>
		class InitExpression : public Expression<R>
		{
		private:
			std::vector<Expression<Lvalue>::Ptr> _exprs;

		public:
			InitExpression(
				std::vector<Expression<Lvalue>::Ptr> exprs)
				: _exprs(std::move(exprs))
			{
			}

			R evaluate(RuntimeContext &context) const override
			{
				if constexpr (std::is_same<void, R>())
				{
					for (const Expression<Lvalue>::Ptr &expr : _exprs)
					{
						expr->evaluate(context);
					}
				}
				else if constexpr (std::is_same<Array, R>() || std::is_same<Tuple, R>())
				{
					InitializerList lst;
					for (const Expression<Lvalue>::Ptr &expr : _exprs)
					{
						lst.push_back(expr->evaluate(context));
					}
					return lst;
				}
			}
		};

		template <typename T>
		class ParamExpression : public Expression<Lvalue>
		{
		private:
			typename Expression<T>::Ptr _expr;

		public:
			ParamExpression(typename Expression<T>::Ptr expr)
				: _expr(std::move(expr))
			{
			}

			Lvalue evaluate(RuntimeContext &context) const override
			{
				return std::make_shared<VariableImpl<T>>(_expr->evaluate(context));
			}
		};

		struct ExpressionBuilderError
		{
			ExpressionBuilderError()
			{
			}
		};

		class TupleInitializationExpression : public Expression<Lvalue>
		{
		private:
			std::vector<Expression<Lvalue>::Ptr> _exprs;

		public:
			TupleInitializationExpression(std::vector<Expression<Lvalue>::Ptr> exprs)
				: _exprs(std::move(exprs))
			{
			}

			Lvalue evaluate(RuntimeContext &context) const override
			{
				Tuple ret;

				for (const Expression<Lvalue>::Ptr &expr : _exprs)
				{
					ret.push_back(expr->evaluate(context));
				}

				return std::make_unique<VariableImpl<Tuple>>(std::move(ret));
			}
		};

		class ClassInitializationExpression : public Expression<Lvalue>
		{
		private:
			std::vector<Expression<Lvalue>::Ptr> _exprs;

		public:
			ClassInitializationExpression(std::vector<Expression<Lvalue>::Ptr> exprs)
				: _exprs(std::move(exprs))
			{
			}

			Lvalue evaluate(RuntimeContext &context) const override
			{
				Class ret;

				for (const Expression<Lvalue>::Ptr &expr : _exprs)
				{
					ret.properties.push_back(expr->evaluate(context));
				}

				return std::make_unique<VariableImpl<Class>>(std::move(ret));
			}
		};

		Expression<Lvalue>::Ptr buildLvalueExpression(TypeHandle typeId, const NodePtr &np, CompilerContext &context);

#define RETURN_EXPRESSION_OF_TYPE(T)              \
	if constexpr (IsConvertible<T, R>::value)     \
	{                                             \
		return build##T##Expression(np, context); \
	}                                             \
	else                                          \
	{                                             \
		throw ExpressionBuilderError();           \
		return ExpressionPtr();                   \
	}

#define CHECK_IDENTIFIER(T1)                                                         \
	if (std::holds_alternative<Identifier>(np->getValue()))                          \
	{                                                                                \
		const Identifier &id = std::get<Identifier>(np->getValue());                 \
		const IdentifierInfo *info = context.find(id.name);                          \
		switch (info->getScope())                                                    \
		{                                                                            \
		case IdentifierScope::GlobalVariable:                                        \
			return std::make_unique<GlobalVariableExpression<R, T1>>(info->index()); \
		case IdentifierScope::LocalVariable:                                         \
			return std::make_unique<LocalVariableExpression<R, T1>>(info->index());  \
		case IdentifierScope::Function:                                              \
			break;                                                                   \
		}                                                                            \
	}

#define CHECK_FUNCTION()                                                                         \
	const IdentifierInfo *info = nullptr;                                                        \
	if (std::holds_alternative<Identifier>(np->getValue()))                                      \
	{                                                                                            \
		const Identifier &id = std::get<Identifier>(np->getValue());                             \
		info = context.find(id.name);                                                            \
	}                                                                                            \
	else if (std::holds_alternative<NodeOperation>(np->getValue()))                              \
	{                                                                                            \
		const NodeOperation no = std::get<NodeOperation>(np->getValue());                        \
		if (no == NodeOperation::Get)                                                            \
		{                                                                                        \
			if (const ClassType *ct = std::get_if<ClassType>(np->getChildren()[0]->getTypeId())) \
			{                                                                                    \
				if (np->getChildren()[1]->isIdentifier())                                        \
				{                                                                                \
					auto name = np->getChildren()[1]->getIdentifier();                           \
					info = context.find(ct->name + "::" + name);                                 \
				}                                                                                \
			}                                                                                    \
		}                                                                                        \
	}                                                                                            \
                                                                                                 \
	if (info)                                                                                    \
	{                                                                                            \
		switch (info->getScope())                                                                \
		{                                                                                        \
		case IdentifierScope::GlobalVariable:                                                    \
		case IdentifierScope::LocalVariable:                                                     \
			break;                                                                               \
		case IdentifierScope::Function:                                                          \
			return std::make_unique<FunctionExpression<R>>(info->index());                       \
		}                                                                                        \
	}

#define CHECK_UNARY_OPERATION(name, T1)                \
	case NodeOperation::name:                          \
		return ExpressionPtr(                          \
			std::make_unique<name##Expression<R, T1>>( \
				ExpressionBuilder<T1>::buildExpression(np->getChildren()[0], context)));

#define CHECK_SIZE_OPERATION()                                                                   \
	case NodeOperation::Size:                                                                    \
		if (std::holds_alternative<ArrayType>(*(np->getChildren()[0]->getTypeId())))             \
		{                                                                                        \
			return ExpressionPtr(                                                                \
				std::make_unique<SizeExpression<R, Larray>>(                                     \
					ExpressionBuilder<Larray>::buildExpression(np->getChildren()[0], context))); \
		}                                                                                        \
		else                                                                                     \
		{                                                                                        \
			return ExpressionPtr(                                                                \
				std::make_unique<ConstantExpression<R, Number>>(1));                             \
		}

#define CHECK_TO_STRING_OPERATION()                                                                           \
	case NodeOperation::ToString:                                                                             \
		if (np->getChildren()[0]->isLvalue())                                                                 \
		{                                                                                                     \
			return ExpressionPtr(std::make_unique<ToStringExpression<R, Lvalue>>(                             \
				ExpressionBuilder<Lvalue>::buildExpression(np->getChildren()[0], context)));                  \
		}                                                                                                     \
		return std::visit(                                                                                    \
			overloaded{                                                                                       \
				[&](SimpleType st) {                                                                          \
					switch (st)                                                                               \
					{                                                                                         \
					case SimpleType::Number:                                                                  \
						return ExpressionPtr(std::make_unique<ToStringExpression<R, Number>>(                 \
							ExpressionBuilder<Number>::buildExpression(np->getChildren()[0], context)));      \
					case SimpleType::String:                                                                  \
						return ExpressionPtr(std::make_unique<ToStringExpression<R, String>>(                 \
							ExpressionBuilder<String>::buildExpression(np->getChildren()[0], context)));      \
					case SimpleType::Void:                                                                    \
						throw ExpressionBuilderError();                                                       \
						return ExpressionPtr();                                                               \
					}                                                                                         \
				},                                                                                            \
				[&](const FunctionType &) {                                                                   \
					return ExpressionPtr(std::make_unique<ToStringExpression<R, Function>>(                   \
						ExpressionBuilder<Function>::buildExpression(np->getChildren()[0], context)));        \
				},                                                                                            \
				[&](const ArrayType &) {                                                                      \
					return ExpressionPtr(std::make_unique<ToStringExpression<R, Array>>(                      \
						ExpressionBuilder<Array>::buildExpression(np->getChildren()[0], context)));           \
				},                                                                                            \
				[&](const TupleType &) {                                                                      \
					return ExpressionPtr(std::make_unique<ToStringExpression<R, Tuple>>(                      \
						ExpressionBuilder<Tuple>::buildExpression(np->getChildren()[0], context)));           \
				},                                                                                            \
				[&](const InitListType &) {                                                                   \
					return ExpressionPtr(std::make_unique<ToStringExpression<R, InitializerList>>(            \
						ExpressionBuilder<InitializerList>::buildExpression(np->getChildren()[0], context))); \
				},                                                                                            \
				[&](const ClassType &) {                                                                      \
					return ExpressionPtr(std::make_unique<ToStringExpression<R, Class>>(                      \
						ExpressionBuilder<Class>::buildExpression(np->getChildren()[0], context)));           \
				}},                                                                                           \
			*np->getChildren()[0]->getTypeId());

#define CHECK_BINARY_OPERATION(name, T1, T2)                                           \
	case NodeOperation::name:                                                          \
		return ExpressionPtr(                                                          \
			std::make_unique<name##Expression<R, T1, T2>>(                             \
				ExpressionBuilder<T1>::buildExpression(np->getChildren()[0], context), \
				ExpressionBuilder<T2>::buildExpression(np->getChildren()[1], context)));

#define CHECK_TERNARY_OPERATION(name, T1, T2, T3)                                      \
	case NodeOperation::name:                                                          \
		return ExpressionPtr(                                                          \
			std::make_unique<name##Expression<R, T1, T2, T3>>(                         \
				ExpressionBuilder<T1>::buildExpression(np->getChildren()[0], context), \
				ExpressionBuilder<T2>::buildExpression(np->getChildren()[1], context), \
				ExpressionBuilder<T3>::buildExpression(np->getChildren()[2], context)));

#define CHECK_COMPARISON_OPERATION(name)                                                         \
	case NodeOperation::name:                                                                    \
		if (                                                                                     \
			np->getChildren()[0]->getTypeId() == TypeRegistry::getNumberHandle() &&              \
			np->getChildren()[1]->getTypeId() == TypeRegistry::getNumberHandle())                \
		{                                                                                        \
			return ExpressionPtr(                                                                \
				std::make_unique<name##Expression<R, Number, Number>>(                           \
					ExpressionBuilder<Number>::buildExpression(np->getChildren()[0], context),   \
					ExpressionBuilder<Number>::buildExpression(np->getChildren()[1], context))); \
		}                                                                                        \
		else                                                                                     \
		{                                                                                        \
			return ExpressionPtr(                                                                \
				std::make_unique<name##Expression<R, String, String>>(                           \
					ExpressionBuilder<String>::buildExpression(np->getChildren()[0], context),   \
					ExpressionBuilder<String>::buildExpression(np->getChildren()[1], context))); \
		}

#define CHECK_INDEX_OPERATION(T, A)                                                            \
	case NodeOperation::Index:                                                                 \
	{                                                                                          \
		const TupleType *tt = std::get_if<TupleType>(np->getChildren()[0]->getTypeId());       \
		if (tt)                                                                                \
		{                                                                                      \
			return ExpressionPtr(                                                              \
				std::make_unique<MemberExpression<R, A, T>>(                                   \
					ExpressionBuilder<A>::buildExpression(np->getChildren()[0], context),      \
					size_t(np->getChildren()[1]->getNumber())));                               \
		}                                                                                      \
		else                                                                                   \
		{                                                                                      \
			const ArrayType *at = std::get_if<ArrayType>(np->getChildren()[0]->getTypeId());   \
			return ExpressionPtr(                                                              \
				std::make_unique<IndexExpression<R, A, T>>(                                    \
					ExpressionBuilder<A>::buildExpression(np->getChildren()[0], context),      \
					ExpressionBuilder<Number>::buildExpression(np->getChildren()[1], context), \
					buildDefaultInitialization(at->innerTypeId)));                             \
		}                                                                                      \
	}

#define CHECK_GET_OPERATION(T, A)                                                                           \
	case NodeOperation::Get:                                                                                \
	{                                                                                                       \
		const ClassType *ct = std::get_if<ClassType>(np->getChildren()[0]->getTypeId());                    \
		if (ct)                                                                                             \
		{                                                                                                   \
			const FunctionType *ft = std::get_if<FunctionType>(np->getTypeId());                            \
			if (ft)                                                                                         \
			{                                                                                               \
				break;                                                                                      \
			}                                                                                               \
			const Property *property = context.getClassProperty(ct, np->getChildren()[1]->getIdentifier()); \
                                                                                                            \
			return ExpressionPtr(                                                                           \
				std::make_unique<ClassMemberExpression<R, A, T>>(                                           \
					ExpressionBuilder<A>::buildExpression(np->getChildren()[0], context),                   \
					property->index));                                                                      \
		}                                                                                                   \
		else                                                                                                \
		{                                                                                                   \
			throw ExpressionBuilderError();                                                                 \
			return ExpressionPtr();                                                                         \
		}                                                                                                   \
	}

#define CHECK_CALL_OPERATION(T)                                                                                               \
	case NodeOperation::Call:                                                                                                 \
	{                                                                                                                         \
		std::vector<Expression<Lvalue>::Ptr> arguments;                                                                       \
		auto &firstChild = np->getChildren()[0];                                                                              \
		const FunctionType *ft = std::get_if<FunctionType>(firstChild->getTypeId());                                          \
		if (std::holds_alternative<NodeOperation>(firstChild->getValue()))                                                    \
		{                                                                                                                     \
			const NodeOperation no = std::get<NodeOperation>(firstChild->getValue());                                         \
			if (no == NodeOperation::Get)                                                                                     \
			{                                                                                                                 \
				arguments.push_back(                                                                                          \
					buildLvalueExpression(firstChild->getChildren()[0]->getTypeId(), firstChild->getChildren()[0], context)); \
			}                                                                                                                 \
		}                                                                                                                     \
		for (size_t i = 1; i < np->getChildren().size(); ++i)                                                                 \
		{                                                                                                                     \
			const NodePtr &child = np->getChildren()[i];                                                                      \
			if (                                                                                                              \
				child->isNodeOperation() &&                                                                                   \
				std::get<NodeOperation>(child->getValue()) == NodeOperation::Param)                                           \
			{                                                                                                                 \
				arguments.push_back(                                                                                          \
					buildLvalueExpression(ft->paramTypeId[i - 1].typeId, child->getChildren()[0], context));                  \
			}                                                                                                                 \
			else                                                                                                              \
			{                                                                                                                 \
				arguments.push_back(                                                                                          \
					ExpressionBuilder<Lvalue>::buildExpression(child, context));                                              \
			}                                                                                                                 \
		}                                                                                                                     \
		return ExpressionPtr(                                                                                                 \
			std::make_unique<CallExpression<R, T>>(                                                                           \
				ExpressionBuilder<Function>::buildExpression(np->getChildren()[0], context),                                  \
				std::move(arguments)));                                                                                       \
	}

		template <typename R>
		class ExpressionBuilder
		{
		private:
			using ExpressionPtr = typename Expression<R>::Ptr;

			static ExpressionPtr buildVoidExpression(const NodePtr &np, CompilerContext &context)
			{
				switch (std::get<NodeOperation>(np->getValue()))
				{
					CHECK_BINARY_OPERATION(Comma, Void, Void);
					CHECK_TERNARY_OPERATION(Ternary, Number, Void, Void);
					CHECK_CALL_OPERATION(Void);
				default:
					throw ExpressionBuilderError();
				}
			}

			static ExpressionPtr buildNumberExpression(const NodePtr &np, CompilerContext &context)
			{
				if (std::holds_alternative<double>(np->getValue()))
				{
					return std::make_unique<ConstantExpression<R, Number>>(
						std::get<double>(np->getValue()));
				}

				CHECK_IDENTIFIER(Lnumber);

				switch (std::get<NodeOperation>(np->getValue()))
				{
					CHECK_UNARY_OPERATION(Postinc, Lnumber);
					CHECK_UNARY_OPERATION(Postdec, Lnumber);
					CHECK_UNARY_OPERATION(Positive, Number);
					CHECK_UNARY_OPERATION(Negative, Number);
					CHECK_UNARY_OPERATION(Bnot, Number);
					CHECK_UNARY_OPERATION(Lnot, Number);
					CHECK_SIZE_OPERATION();
					CHECK_BINARY_OPERATION(Add, Number, Number);
					CHECK_BINARY_OPERATION(Sub, Number, Number);
					CHECK_BINARY_OPERATION(Mul, Number, Number);
					CHECK_BINARY_OPERATION(Div, Number, Number);
					CHECK_BINARY_OPERATION(Idiv, Number, Number);
					CHECK_BINARY_OPERATION(Mod, Number, Number);
					CHECK_BINARY_OPERATION(Band, Number, Number);
					CHECK_BINARY_OPERATION(Bor, Number, Number);
					CHECK_BINARY_OPERATION(Bxor, Number, Number);
					CHECK_BINARY_OPERATION(Bsl, Number, Number);
					CHECK_BINARY_OPERATION(Bsr, Number, Number);
					CHECK_COMPARISON_OPERATION(Eq);
					CHECK_COMPARISON_OPERATION(Ne);
					CHECK_COMPARISON_OPERATION(Lt);
					CHECK_COMPARISON_OPERATION(Gt);
					CHECK_COMPARISON_OPERATION(Le);
					CHECK_COMPARISON_OPERATION(Ge);
					CHECK_BINARY_OPERATION(Comma, Void, Number);
					CHECK_BINARY_OPERATION(Land, Number, Number);
					CHECK_BINARY_OPERATION(Lor, Number, Number);
					CHECK_INDEX_OPERATION(Number, Array);
					CHECK_TERNARY_OPERATION(Ternary, Number, Number, Number);
					CHECK_CALL_OPERATION(Number);
				default:
					throw ExpressionBuilderError();
				}
			}

			static ExpressionPtr buildLnumberExpression(const NodePtr &np, CompilerContext &context)
			{
				CHECK_IDENTIFIER(Lnumber);

				switch (std::get<NodeOperation>(np->getValue()))
				{
					CHECK_UNARY_OPERATION(Preinc, Lnumber);
					CHECK_UNARY_OPERATION(Predec, Lnumber);
					CHECK_BINARY_OPERATION(Assign, Lnumber, Number);
					CHECK_BINARY_OPERATION(AddAssign, Lnumber, Number);
					CHECK_BINARY_OPERATION(SubAssign, Lnumber, Number);
					CHECK_BINARY_OPERATION(MulAssign, Lnumber, Number);
					CHECK_BINARY_OPERATION(DivAssign, Lnumber, Number);
					CHECK_BINARY_OPERATION(IdivAssign, Lnumber, Number);
					CHECK_BINARY_OPERATION(ModAssign, Lnumber, Number);
					CHECK_BINARY_OPERATION(BandAssign, Lnumber, Number);
					CHECK_BINARY_OPERATION(BorAssign, Lnumber, Number);
					CHECK_BINARY_OPERATION(BxorAssign, Lnumber, Number);
					CHECK_BINARY_OPERATION(BslAssign, Lnumber, Number);
					CHECK_BINARY_OPERATION(BsrAssign, Lnumber, Number);
					CHECK_BINARY_OPERATION(Comma, Void, Lnumber);
					CHECK_INDEX_OPERATION(Lnumber, Larray);
					CHECK_GET_OPERATION(Lnumber, Lclass);
					CHECK_TERNARY_OPERATION(Ternary, Number, Lnumber, Lnumber);
				default:
					throw ExpressionBuilderError();
				}
			}

			static ExpressionPtr buildStringExpression(const NodePtr &np, CompilerContext &context)
			{
				if (std::holds_alternative<std::string>(np->getValue()))
				{
					return std::make_unique<ConstantExpression<R, String>>(
						std::make_shared<std::string>(std::get<std::string>(np->getValue())));
				}

				CHECK_IDENTIFIER(Lstring);

				switch (std::get<NodeOperation>(np->getValue()))
				{
					CHECK_TO_STRING_OPERATION();
					CHECK_BINARY_OPERATION(Concat, String, String);
					CHECK_BINARY_OPERATION(Comma, Void, String);
					CHECK_INDEX_OPERATION(String, Array);
					CHECK_TERNARY_OPERATION(Ternary, Number, String, String);
					CHECK_CALL_OPERATION(String);
				default:
					throw ExpressionBuilderError();
				}
			}

			static ExpressionPtr buildLstringExpression(const NodePtr &np, CompilerContext &context)
			{
				CHECK_IDENTIFIER(Lstring);

				switch (std::get<NodeOperation>(np->getValue()))
				{
					CHECK_BINARY_OPERATION(Assign, Lstring, String);
					CHECK_BINARY_OPERATION(ConcatAssign, Lstring, String);
					CHECK_BINARY_OPERATION(Comma, Void, Lstring);
					CHECK_INDEX_OPERATION(Lstring, Larray);
					CHECK_GET_OPERATION(Lstring, Lclass);
					CHECK_TERNARY_OPERATION(Ternary, Number, Lstring, Lstring);
				default:
					throw ExpressionBuilderError();
				}
			}

			static ExpressionPtr buildArrayExpression(const NodePtr &np, CompilerContext &context)
			{
				CHECK_IDENTIFIER(Larray);

				switch (std::get<NodeOperation>(np->getValue()))
				{
					CHECK_BINARY_OPERATION(Comma, Void, Array);
					CHECK_INDEX_OPERATION(Array, Array);
					CHECK_TERNARY_OPERATION(Ternary, Number, Array, Array);
					CHECK_CALL_OPERATION(Array);
				default:
					throw ExpressionBuilderError();
				}
			}

			static ExpressionPtr buildLarrayExpression(const NodePtr &np, CompilerContext &context)
			{
				CHECK_IDENTIFIER(Larray);

				switch (std::get<NodeOperation>(np->getValue()))
				{
					CHECK_BINARY_OPERATION(Assign, Larray, Array);
					CHECK_BINARY_OPERATION(Comma, Void, Larray);
					CHECK_INDEX_OPERATION(Larray, Larray);
					CHECK_GET_OPERATION(Larray, Lclass);
					CHECK_TERNARY_OPERATION(Ternary, Number, Larray, Larray);
				default:
					throw ExpressionBuilderError();
				}
			}

			static ExpressionPtr buildFunctionExpression(const NodePtr &np, CompilerContext &context)
			{
				CHECK_IDENTIFIER(Lfunction);
				CHECK_FUNCTION();

				switch (std::get<NodeOperation>(np->getValue()))
				{
					CHECK_BINARY_OPERATION(Comma, Void, Function);
					CHECK_INDEX_OPERATION(Function, Array);
					CHECK_TERNARY_OPERATION(Ternary, Number, Function, Function);
					CHECK_GET_OPERATION(Lfunction, Lclass);
					CHECK_CALL_OPERATION(Function);
				default:
					throw ExpressionBuilderError();
				}
			}

			static ExpressionPtr buildLfunctionExpression(const NodePtr &np, CompilerContext &context)
			{
				CHECK_IDENTIFIER(Lfunction);

				switch (std::get<NodeOperation>(np->getValue()))
				{
					CHECK_BINARY_OPERATION(Assign, Lfunction, Function);
					CHECK_BINARY_OPERATION(Comma, Void, Lfunction);
					CHECK_INDEX_OPERATION(Lfunction, Larray);
					CHECK_TERNARY_OPERATION(Ternary, Number, Lfunction, Lfunction);
				default:
					throw ExpressionBuilderError();
				}
			}

			static ExpressionPtr buildTupleExpression(const NodePtr &np, CompilerContext &context)
			{
				CHECK_IDENTIFIER(Ltuple);

				switch (std::get<NodeOperation>(np->getValue()))
				{
					CHECK_BINARY_OPERATION(Comma, Void, Tuple);
					CHECK_INDEX_OPERATION(Tuple, Array);
					CHECK_TERNARY_OPERATION(Ternary, Number, Tuple, Tuple);
					CHECK_CALL_OPERATION(Tuple);
				default:
					throw ExpressionBuilderError();
				}
			}

			static ExpressionPtr buildLtupleExpression(const NodePtr &np, CompilerContext &context)
			{
				CHECK_IDENTIFIER(Ltuple);

				switch (std::get<NodeOperation>(np->getValue()))
				{
					CHECK_BINARY_OPERATION(Assign, Ltuple, Tuple);
					CHECK_BINARY_OPERATION(Comma, Void, Ltuple);
					CHECK_INDEX_OPERATION(Ltuple, Larray);
					CHECK_GET_OPERATION(Ltuple, Lclass);
					CHECK_TERNARY_OPERATION(Ternary, Number, Ltuple, Ltuple);
					CHECK_CALL_OPERATION(Ltuple);
				default:
					throw ExpressionBuilderError();
				}
			}

			static ExpressionPtr buildClassExpression(const NodePtr &np, CompilerContext &context)
			{
				CHECK_IDENTIFIER(Lclass);

				switch (std::get<NodeOperation>(np->getValue()))
				{
					CHECK_BINARY_OPERATION(Comma, Void, Class);
					CHECK_TERNARY_OPERATION(Ternary, Number, Class, Class);
					CHECK_CALL_OPERATION(Class);
				default:
					throw ExpressionBuilderError();
				}
			}

			static ExpressionPtr buildLclassExpression(const NodePtr &np, CompilerContext &context)
			{
				CHECK_IDENTIFIER(Lclass);

				switch (std::get<NodeOperation>(np->getValue()))
				{
					CHECK_BINARY_OPERATION(Assign, Lclass, Class);
					CHECK_BINARY_OPERATION(Comma, Void, Lclass);
					CHECK_TERNARY_OPERATION(Ternary, Number, Lclass, Lclass);
					CHECK_GET_OPERATION(Lclass, Lclass);
					CHECK_CALL_OPERATION(Lclass);
				default:
					throw ExpressionBuilderError();
				}
			}

			static ExpressionPtr buildInitializerListExpression(const NodePtr &np, CompilerContext &context)
			{
				switch (std::get<NodeOperation>(np->getValue()))
				{
				case NodeOperation::Init:
				{
					std::vector<Expression<Lvalue>::Ptr> exprs;
					exprs.reserve(np->getChildren().size());
					for (const NodePtr &child : np->getChildren())
					{
						exprs.emplace_back(buildLvalueExpression(child->getTypeId(), child, context));
					}
					return std::make_unique<InitExpression<R>>(std::move(exprs));
				}
					CHECK_BINARY_OPERATION(Comma, Void, InitializerList);
					CHECK_TERNARY_OPERATION(Ternary, Number, InitializerList, InitializerList);
				default:
					throw ExpressionBuilderError();
				}
			}

		public:
			static ExpressionPtr buildExpression(const NodePtr &np, CompilerContext &context)
			{
				return std::visit(
					overloaded{
						[&](SimpleType st)
						{
							switch (st)
							{
							case SimpleType::Number:
								if (np->isLvalue())
								{
									RETURN_EXPRESSION_OF_TYPE(Lnumber);
								}
								else
								{
									RETURN_EXPRESSION_OF_TYPE(Number);
								}
							case SimpleType::String:
								if (np->isLvalue())
								{
									RETURN_EXPRESSION_OF_TYPE(Lstring);
								}
								else
								{
									RETURN_EXPRESSION_OF_TYPE(String);
								}
							case SimpleType::Void:
								RETURN_EXPRESSION_OF_TYPE(Void);
							}
						},
						[&](const FunctionType &ft)
						{
							if (np->isLvalue())
							{
								RETURN_EXPRESSION_OF_TYPE(Lfunction);
							}
							else
							{
								RETURN_EXPRESSION_OF_TYPE(Function);
							}
						},
						[&](const ArrayType &at)
						{
							if (np->isLvalue())
							{
								RETURN_EXPRESSION_OF_TYPE(Larray);
							}
							else
							{
								RETURN_EXPRESSION_OF_TYPE(Array);
							}
						},
						[&](const TupleType &tt)
						{
							if (np->isLvalue())
							{
								RETURN_EXPRESSION_OF_TYPE(Ltuple);
							}
							else
							{
								RETURN_EXPRESSION_OF_TYPE(Tuple);
							}
						},
						[&](const InitListType &ilt)
						{
							RETURN_EXPRESSION_OF_TYPE(InitializerList);
						},
						[&](const ClassType &ct)
						{
							if (np->isLvalue())
							{
								RETURN_EXPRESSION_OF_TYPE(Lclass);
							}
							else
							{
								RETURN_EXPRESSION_OF_TYPE(Class);
							}
						}},
					*np->getTypeId());
			}

			static Expression<Lvalue>::Ptr buildParamExpression(const NodePtr &np, CompilerContext &context)
			{
				return std::make_unique<ParamExpression<R>>(
					ExpressionBuilder<R>::buildExpression(np, context));
			}
		};

#undef CHECK_CALL_OPERATION
#undef CHECK_INDEX_OPERATION
#undef CHECK_COMPARISON_OPERATION
#undef CHECK_TERNARY_OPERATION
#undef CHECK_BINARY_OPERATION
#undef CHECK_TO_STRING_OPERATION
#undef CHECK_SIZE_OPERATION
#undef CHECK_UNARY_OPERATION
#undef CHECK_FUNCTION
#undef CHECK_IDENTIFIER
#undef RETURN_EXPRESSION_OF_TYPE

		Expression<Lvalue>::Ptr buildLvalueExpression(TypeHandle typeId, const NodePtr &np, CompilerContext &context)
		{
			return std::visit(
				overloaded{
					[&](SimpleType st)
					{
						switch (st)
						{
						case SimpleType::Number:
							return ExpressionBuilder<Number>::buildParamExpression(np, context);
						case SimpleType::String:
							return ExpressionBuilder<String>::buildParamExpression(np, context);
						case SimpleType::Void:
							throw ExpressionBuilderError();
							return Expression<Lvalue>::Ptr();
						}
					},
					[&](const FunctionType &)
					{
						return ExpressionBuilder<Function>::buildParamExpression(np, context);
					},
					[&](const ArrayType &)
					{
						return ExpressionBuilder<Array>::buildParamExpression(np, context);
					},
					[&](const TupleType &)
					{
						return ExpressionBuilder<Tuple>::buildParamExpression(np, context);
					},
					[&](const InitListType &)
					{
						throw ExpressionBuilderError();
						return Expression<Lvalue>::Ptr();
					},
					[&](const ClassType &)
					{
						return ExpressionBuilder<Class>::buildParamExpression(np, context);
					}},
				*typeId);
		}

		class EmptyExpression : public Expression<void>
		{
			void evaluate(RuntimeContext &) const override
			{
			}
		};

		template <typename R>
		typename Expression<R>::Ptr buildExpression(TypeHandle typeId, CompilerContext &context, TokensIterator &it, bool allow_comma)
		{
			size_t line_number = it->getLineNumber();
			size_t char_index = it->getCharIndex();

			try
			{
				NodePtr np = parseExpressionTree(context, it, typeId, allow_comma);

				if constexpr (std::is_same<void, R>::value)
				{
					if (!np)
					{
						return std::make_unique<EmptyExpression>();
					}
				}
				if constexpr (std::is_same<R, Lvalue>::value)
				{
					return buildLvalueExpression(
						typeId,
						np,
						context);
				}
				else
				{
					return ExpressionBuilder<R>::buildExpression(
						np,
						context);
				}
			}
			catch (const ExpressionBuilderError &)
			{
				throw compilerError("Expression building failed", line_number, char_index);
			}
		}

		template <typename T>
		class DefaultInitializationExpression : public Expression<Lvalue>
		{
		public:
			Lvalue evaluate(RuntimeContext &context) const override
			{
				return std::make_shared<VariableImpl<T>>(T{});
			}
		};
	}

	Expression<void>::Ptr buildVoidExpression(CompilerContext &context, TokensIterator &it)
	{
		return buildExpression<void>(TypeRegistry::getVoidHandle(), context, it, true);
	}

	Expression<Number>::Ptr buildNumberExpression(CompilerContext &context, TokensIterator &it)
	{
		return buildExpression<Number>(TypeRegistry::getNumberHandle(), context, it, true);
	}

	Expression<Lvalue>::Ptr buildInitializationExpression(
		CompilerContext &context,
		TokensIterator &it,
		TypeHandle typeId,
		bool allow_comma)
	{
		return buildExpression<Lvalue>(typeId, context, it, allow_comma);
	}

	Expression<Lvalue>::Ptr buildDefaultInitialization(TypeHandle typeId)
	{
		return std::visit(
			overloaded{
				[&](SimpleType st)
				{
					switch (st)
					{
					case SimpleType::Number:
						return Expression<Lvalue>::Ptr(std::make_unique<DefaultInitializationExpression<Number>>());
					case SimpleType::String:
						return Expression<Lvalue>::Ptr(std::make_unique<DefaultInitializationExpression<String>>());
					case SimpleType::Void:
						return Expression<Lvalue>::Ptr(nullptr); //cannot happen
					}
				},
				[&](const FunctionType &ft)
				{
					return Expression<Lvalue>::Ptr(std::make_unique<DefaultInitializationExpression<Function>>());
				},
				[&](const ArrayType &at)
				{
					return Expression<Lvalue>::Ptr(std::make_unique<DefaultInitializationExpression<Array>>());
				},
				[&](const TupleType &tt)
				{
					std::vector<Expression<Lvalue>::Ptr> exprs;

					exprs.reserve(tt.innerTypeId.size());

					for (TypeHandle it : tt.innerTypeId)
					{
						exprs.emplace_back(buildDefaultInitialization(it));
					}

					return Expression<Lvalue>::Ptr(
						std::make_unique<TupleInitializationExpression>(std::move(exprs)));
				},
				[&](const InitListType &ilt)
				{
					//cannot happen
					return Expression<Lvalue>::Ptr();
				},
				[&](const ClassType &ct)
				{
					std::vector<Expression<Lvalue>::Ptr> exprs(ct.properties.size());

					for (auto &it : ct.properties)
					{
						auto &property = it.second;
						exprs.at(property.index) = buildDefaultInitialization(property.type);
					}

					return Expression<Lvalue>::Ptr(
						std::make_unique<ClassInitializationExpression>(std::move(exprs)));
				}},
			*typeId);
	}
}
