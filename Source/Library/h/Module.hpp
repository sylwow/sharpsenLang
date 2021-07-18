#pragma once
#include <functional>
#include <tuple>
#include <type_traits>
#include <utility>
#include <iostream>

#include "Variable.hpp"
#include "RuntimeContext.hpp"

namespace sharpsenLang
{
	namespace details
	{
		template <typename R, typename Unpacked, typename Left>
		struct Unpacker;

		template <typename R, typename... Unpacked, typename Left0, typename... Left>
		struct Unpacker<R, std::tuple<Unpacked...>, std::tuple<Left0, Left...>>
		{
			R operator()(
				RuntimeContext &ctx,
				const std::function<R(Unpacked..., Left0, Left...)> &f,
				std::tuple<Unpacked...> t) const
			{
				using NextUnpacker = Unpacker<R, std::tuple<Unpacked..., Left0>, std::tuple<Left...>>;
				if constexpr (std::is_convertible<const std::string &, Left0>::value)
				{
					return NextUnpacker()(
						ctx,
						f,
						std::tuple_cat(
							std::move(t),
							std::tuple<Left0>(
								*ctx.local(
										-1 - int(sizeof...(Unpacked)))
									 ->staticPointerDowncast<Lstring>()
									 ->value)));
				}
				else
				{
					static_assert(std::is_convertible<Number, Left0>::value);
					return NextUnpacker()(
						ctx,
						f,
						std::tuple_cat(
							std::move(t),
							std::tuple<Left0>(
								ctx.local(
									   -1 - int(sizeof...(Unpacked)))
									->staticPointerDowncast<Lnumber>()
									->value)));
				}
			}
		};

		template <typename R, typename... Unpacked>
		struct Unpacker<R, std::tuple<Unpacked...>, std::tuple<>>
		{
			R operator()(
				RuntimeContext &ctx,
				const std::function<R(Unpacked...)> &f,
				std::tuple<Unpacked...> t) const
			{
				return std::apply(f, t);
			}
		};

		template <typename R, typename... Args>
		Function createExternalFunction(std::function<R(Args...)> f)
		{
			return [f = std::move(f)](RuntimeContext &ctx)
			{
				if constexpr (std::is_same<R, void>::value)
				{
					Unpacker<R, std::tuple<>, std::tuple<Args...>>()(ctx, f, std::tuple<>());
				}
				else
				{
					R retval = Unpacker<R, std::tuple<>, std::tuple<Args...>>()(ctx, f, std::tuple<>());
					if constexpr (std::is_convertible<R, std::string>::value)
					{
						ctx.retval() = std::make_shared<VariableImpl<String>>(std::make_shared<std::string>(std::move(retval)));
					}
					else
					{
						static_assert(std::is_convertible<R, Number>::value);
						ctx.retval() = std::make_shared<VariableImpl<Number>>(retval);
					}
				}
			};
		}

		template <typename T>
		struct RetvalDeclaration
		{
			static constexpr const char *result()
			{
				if constexpr (std::is_same<T, void>::value)
				{
					return "void";
				}
				else if constexpr (std::is_convertible<T, std::string>::value)
				{
					return "string";
				}
				else
				{
					static_assert(std::is_convertible<T, Number>::value);
					return "number";
				}
			}
		};

		template <typename T>
		struct ArgumentDeclaration
		{
			static constexpr const char *result()
			{
				if constexpr (std::is_convertible<const std::string &, T>::value)
				{
					return "string";
				}
				else
				{
					static_assert(std::is_convertible<Number, T>::value);
					return "number";
				}
			}
		};

		struct FunctionArgumentString
		{
			std::string str;
			FunctionArgumentString(const char *p)
				: str(p)
			{
			}

			FunctionArgumentString &operator+=(const FunctionArgumentString &oth)
			{
				str += ", ";
				str += oth.str;
				return *this;
			}
		};

		template <typename R, typename... Args>
		std::string createFunctionDeclaration(const char *name)
		{
			if constexpr (sizeof...(Args) == 0)
			{
				return std::string("function ") + RetvalDeclaration<R>::result() + " " + name + "()";
			}
			else
			{
				return std::string("function ") + RetvalDeclaration<R>::result() + " " + name +
					   "(" +
					   (FunctionArgumentString(ArgumentDeclaration<Args>::result()) += ...).str +
					   ")";
			}
		}

		inline VariablePtr toVariable(Number n)
		{
			return std::make_shared<VariableImpl<Number>>(n);
		}

		inline VariablePtr toVariable(std::string str)
		{
			return std::make_shared<VariableImpl<String>>(std::make_shared<std::string>(std::move(str)));
		}

		template <typename T>
		T moveFromVariable(const VariablePtr &v)
		{
			if constexpr (std::is_same<T, std::string>::value)
			{
				return std::move(*v->staticPointerDowncast<Lstring>()->value);
			}
			else
			{
				static_assert(std::is_same<Number, T>::value);
				return v->staticPointerDowncast<Lnumber>()->value;
			}
		}
	}

	class ModuleImpl;

	class Module
	{
	private:
		std::unique_ptr<ModuleImpl> _impl;
		void addExternalFunctionImpl(std::string declaration, Function f);
		void addPublicFunctionDeclaration(std::string declaration, std::string name, std::shared_ptr<Function> fptr);
		RuntimeContext *getRuntimeContext();

	public:
		Module();

		template <typename R, typename... Args>
		void addExternalFunction(const char *name, std::function<R(Args...)> f)
		{
			addExternalFunctionImpl(
				details::createFunctionDeclaration<R, Args...>(name),
				details::createExternalFunction(std::move(f)));
		}

		template <typename R, typename... Args>
		auto createPublicFunctionCaller(std::string name)
		{
			std::shared_ptr<Function> fptr = std::make_shared<Function>();
			std::string decl = details::createFunctionDeclaration<R, Args...>(name.c_str());
			addPublicFunctionDeclaration(std::move(decl), std::move(name), fptr);

			return [this, fptr](Args... args)
			{
				if constexpr (std::is_same<R, void>::value)
				{
					getRuntimeContext()->call(
						*fptr,
						{details::toVariable(std::move(args))...});
				}
				else
				{
					return details::moveFromVariable<R>(getRuntimeContext()->call(
						*fptr,
						{details::toVariable(args)...}));
				}
			};
		}

		void load(const char *path);
		bool tryLoad(const char *path, std::ostream *err = nullptr) noexcept;

		void resetGlobals();

		~Module();
	};
}
