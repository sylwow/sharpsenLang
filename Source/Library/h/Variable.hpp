#pragma once
#include <memory>
#include <deque>
#include <vector>
#include <functional>
#include <string>

namespace sharpsenLang
{

	class Variable;

	using VariablePtr = std::shared_ptr<Variable>;

	template <class T>
	class VariableImpl;

	class RuntimeContext;

	using Void = void;
	using Number = double;
	using String = std::shared_ptr<std::string>;
	using Array = std::deque<VariablePtr>;
	using Function = std::function<void(RuntimeContext &)>;
	using Tuple = Array;
	using InitializerList = Array;

	struct Class
	{
		std::vector<VariablePtr> properties;
	};

	using Lvalue = VariablePtr;
	using Lnumber = std::shared_ptr<VariableImpl<Number>>;
	using Lstring = std::shared_ptr<VariableImpl<String>>;
	using Larray = std::shared_ptr<VariableImpl<Array>>;
	using Lfunction = std::shared_ptr<VariableImpl<Function>>;
	using Ltuple = std::shared_ptr<VariableImpl<Tuple>>;
	using Lclass = std::shared_ptr<VariableImpl<Class>>;

	class Variable : public std::enable_shared_from_this<Variable>
	{
	private:
		Variable(const Variable &) = delete;
		void operator=(const Variable &) = delete;

	protected:
		Variable() = default;

	public:
		virtual ~Variable() = default;

		template <typename T>
		T staticPointerDowncast()
		{
			return std::static_pointer_cast<
				VariableImpl<typename T::element_type::ValueType>>(shared_from_this());
		}

		virtual VariablePtr clone() const = 0;

		virtual String toString() const = 0;
	};

	template <typename T>
	class VariableImpl : public Variable
	{
	public:
		using ValueType = T;

		ValueType value;

		VariableImpl(ValueType value);

		VariablePtr clone() const override;

		String toString() const override;
	};

	Number cloneVariableValue(Number value);
	String cloneVariableValue(const String &value);
	Function cloneVariableValue(const Function &value);
	Array cloneVariableValue(const Array &value);
	Class cloneVariableValue(const Class &value);

	template <class T>
	T cloneVariableValue(const std::shared_ptr<VariableImpl<T>> &v)
	{
		return cloneVariableValue(v->value);
	}

	String convertToString(Number value);
	String convertToString(const String &value);
	String convertToString(const Function &value);
	String convertToString(const Array &value);
	String convertToString(const Lvalue &var);
	String convertToString(const Class &var);
}
