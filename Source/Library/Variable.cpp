#include "Variable.hpp"

namespace sharpsenLang
{
	namespace
	{
		String fromStdString(std::string str)
		{
			return std::make_shared<std::string>(std::move(str));
		}
	}



	template <typename T>
	VariableImpl<T>::VariableImpl(T value) : value(std::move(value))
	{
	}

	template <typename T>
	VariablePtr VariableImpl<T>::clone() const
	{
		return std::make_shared<VariableImpl<T>>(cloneVariableValue(value));
	}

	template <typename T>
	String VariableImpl<T>::toString() const
	{
		return convertToString(value);
	}

	template class VariableImpl<Number>;
	template class VariableImpl<String>;
	template class VariableImpl<Function>;
	template class VariableImpl<Array>;
	template class VariableImpl<Class>;

	Number cloneVariableValue(Number value)
	{
		return value;
	}

	String cloneVariableValue(const String &value)
	{
		return value;
	}

	Function cloneVariableValue(const Function &value)
	{
		return value;
	}

	Array cloneVariableValue(const Array &value)
	{
		Array ret;
		for (const VariablePtr &v : value)
		{
			ret.push_back(v->clone());
		}
		return ret;
	}

	Class cloneVariableValue(const Class &value)
	{
		Class ret;
		for (const VariablePtr &v : value.properties)
		{
			ret.properties.push_back(v->clone());
		}
		return ret;
	}

	String convertToString(Number value)
	{
		if (value == int(value))
		{
			return fromStdString(std::to_string(int(value)));
		}
		else
		{
			return fromStdString(std::to_string(value));
		}
	}

	String convertToString(const String &value)
	{
		return value;
	}

	String convertToString(const Function &value)
	{
		return fromStdString("FUNCTION");
	}

	String convertToString(const Array &value)
	{
		std::string ret = "[";
		const char *separator = "";
		for (const VariablePtr &v : value)
		{
			ret += separator;
			ret += *(v->toString());
			separator = ", ";
		}
		ret += "]";
		return fromStdString(std::move(ret));
	}

	String convertToString(const Class &value)
	{
		return fromStdString("CLASS");
	}

	String convertToString(const Lvalue &var)
	{
		return var->toString();
	}
}