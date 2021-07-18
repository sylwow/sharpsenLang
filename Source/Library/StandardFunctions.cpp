#include <iostream>
#include <string>
#include <cmath>
#include <cstdlib>
#include <ctime>

#include "StandardFunctions.hpp"
#include "Module.hpp"

namespace sharpsenLang
{

	void addMathFunctions(Module &m)
	{
		m.addExternalFunction("sin", std::function<Number(Number)>(
										 [](Number x)
										 {
											 return std::sin(x);
										 }));

		m.addExternalFunction("cos", std::function<Number(Number)>(
										 [](Number x)
										 {
											 return std::cos(x);
										 }));

		m.addExternalFunction("tan", std::function<Number(Number)>(
										 [](Number x)
										 {
											 return std::tan(x);
										 }));

		m.addExternalFunction("log", std::function<Number(Number)>(
										 [](Number x)
										 {
											 return std::log(x);
										 }));

		m.addExternalFunction("exp", std::function<Number(Number)>(
										 [](Number x)
										 {
											 return std::exp(x);
										 }));

		m.addExternalFunction("pow", std::function<Number(Number, Number)>(
										 [](Number x, Number y)
										 {
											 return std::pow(x, y);
										 }));

		srand((unsigned int)time(0));

		m.addExternalFunction("rnd", std::function<Number(Number)>(
										 [](Number x)
										 {
											 return rand() % int(x);
										 }));
	}

	void addStringFunctions(Module &m)
	{
		m.addExternalFunction("strlen", std::function<Number(const std::string &)>(
											[](const std::string &str)
											{
												return str.size();
											}));

		m.addExternalFunction("substr", std::function<std::string(const std::string &, Number, Number)>(
											[](const std::string &str, Number from, Number count)
											{
												return str.substr(size_t(from), size_t(count));
											}));
	}

	void addTraceFunctions(Module &m)
	{
		m.addExternalFunction("trace", std::function<void(const std::string &)>(
										   [](const std::string &str)
										   {
											   std::cout << str << std::endl;
										   }));
	}

	void addStandardFunctions(Module &m)
	{
		addMathFunctions(m);
		addStringFunctions(m);
		addTraceFunctions(m);
	}
}
