#include <iostream>
#include <string>
#include <cmath>
#include <cstdlib>
#include <ctime>

#include "StandardFunctions.hpp"
#include "Module.hpp"


namespace sharpsenLang {

	void add_math_functions(Module& m) {
		m.add_external_function("sin", std::function<Number(Number)>(
			[](Number x) {
				return std::sin(x);
			}
		));
		
		m.add_external_function("cos", std::function<Number(Number)>(
			[](Number x) {
				return std::cos(x);
			}
		));
		
		m.add_external_function("tan", std::function<Number(Number)>(
			[](Number x) {
				return std::tan(x);
			}
		));
		
		m.add_external_function("log", std::function<Number(Number)>(
			[](Number x) {
				return std::log(x);
			}
		));
		
		m.add_external_function("exp", std::function<Number(Number)>(
			[](Number x) {
				return std::exp(x);
			}
		));
		
		m.add_external_function("pow", std::function<Number(Number, Number)>(
			[](Number x, Number y) {
				return std::pow(x, y);
			}
		));
		
		srand((unsigned int)time(0));
		
		m.add_external_function("rnd", std::function<Number(Number)>(
			[](Number x) {
				return rand() % int(x);
			}
		));
	}
	
	void add_string_functions(Module& m) {
		m.add_external_function("strlen", std::function<Number(const std::string&)>(
			[](const std::string& str) {
				return str.size();
			}
		));
		
		m.add_external_function("substr", std::function<std::string(const std::string&, Number, Number)>(
			[](const std::string& str, Number from, Number count) {
				return str.substr(size_t(from), size_t(count));
			}
		));
	}
	
	void add_trace_functions(Module& m) {
		m.add_external_function("trace", std::function<void(const std::string&)>(
			[](const std::string& str) {
				std::cout << str << std::endl;
			}
		));
	}
	
	void add_standard_functions(Module& m) {
		add_math_functions(m);
		add_string_functions(m);
		add_trace_functions(m);
	}

}
