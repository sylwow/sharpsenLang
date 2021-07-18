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
		struct unpacker;

		template <typename R, typename... Unpacked, typename Left0, typename... Left>
		struct unpacker<R, std::tuple<Unpacked...>, std::tuple<Left0, Left...>>
		{
			R operator()(
				RuntimeContext &ctx,
				const std::function<R(Unpacked..., Left0, Left...)> &f,
				std::tuple<Unpacked...> t) const
			{
				using next_unpacker = unpacker<R, std::tuple<Unpacked..., Left0>, std::tuple<Left...>>;
				if constexpr (std::is_convertible<const std::string &, Left0>::value)
				{
					return next_unpacker()(
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
					return next_unpacker()(
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
		struct unpacker<R, std::tuple<Unpacked...>, std::tuple<>>
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
		Function create_external_function(std::function<R(Args...)> f)
		{
			return [f = std::move(f)](RuntimeContext &ctx)
			{
				if constexpr (std::is_same<R, void>::value)
				{
					unpacker<R, std::tuple<>, std::tuple<Args...>>()(ctx, f, std::tuple<>());
				}
				else
				{
					R retval = unpacker<R, std::tuple<>, std::tuple<Args...>>()(ctx, f, std::tuple<>());
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
		struct retval_declaration
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
		struct argument_declaration
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

		struct function_argument_string
		{
			std::string str;
			function_argument_string(const char *p)
				: str(p)
			{
			}

			function_argument_string &operator+=(const function_argument_string &oth)
			{
				str += ", ";
				str += oth.str;
				return *this;
			}
		};

		template <typename R, typename... Args>
		std::string create_function_declaration(const char *name)
		{
			if constexpr (sizeof...(Args) == 0)
			{
				return std::string("function ") + retval_declaration<R>::result() + " " + name + "()";
			}
			else
			{
				return std::string("function ") + retval_declaration<R>::result() + " " + name +
					   "(" +
					   (function_argument_string(argument_declaration<Args>::result()) += ...).str +
					   ")";
			}
		}

		inline VariablePtr to_variable(Number n)
		{
			return std::make_shared<VariableImpl<Number>>(n);
		}

		inline VariablePtr to_variable(std::string str)
		{
			return std::make_shared<VariableImpl<String>>(std::make_shared<std::string>(std::move(str)));
		}

		template <typename T>
		T move_from_variable(const VariablePtr &v)
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

	class module_impl;

	class Module
	{
	private:
		std::unique_ptr<module_impl> _impl;
		void add_external_function_impl(std::string declaration, Function f);
		void add_public_function_declaration(std::string declaration, std::string name, std::shared_ptr<Function> fptr);
		RuntimeContext *get_runtime_context();

	public:
		Module();

		template <typename R, typename... Args>
		void add_external_function(const char *name, std::function<R(Args...)> f)
		{
			add_external_function_impl(
				details::create_function_declaration<R, Args...>(name),
				details::create_external_function(std::move(f)));
		}

		template <typename R, typename... Args>
		auto create_public_function_caller(std::string name)
		{
			std::shared_ptr<Function> fptr = std::make_shared<Function>();
			std::string decl = details::create_function_declaration<R, Args...>(name.c_str());
			add_public_function_declaration(std::move(decl), std::move(name), fptr);

			return [this, fptr](Args... args)
			{
				if constexpr (std::is_same<R, void>::value)
				{
					get_runtime_context()->call(
						*fptr,
						{details::to_variable(std::move(args))...});
				}
				else
				{
					return details::move_from_variable<R>(get_runtime_context()->call(
						*fptr,
						{details::to_variable(args)...}));
				}
			};
		}

		void load(const char *path);
		bool try_load(const char *path, std::ostream *err = nullptr) noexcept;

		void reset_globals();

		~Module();
	};
}
