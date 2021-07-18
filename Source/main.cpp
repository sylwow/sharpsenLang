#include <iostream>
#include "Module.hpp"
#include "StandardFunctions.hpp"

int main() {
	std::string path = __FILE__;
	path = path.substr(0, path.find_last_of("/\\") + 1) + "test.stk";
	
	using namespace sharpsenLang;
	
	Module m;
	
	add_standard_functions(m);
	
	m.add_external_function("greater", std::function<Number(Number, Number)>([](Number x, Number y){
		return x > y;
	}));
	
	auto s_main = m.create_public_function_caller<void>("main");
	
	if (m.try_load(path.c_str(), &std::cerr)) {
		s_main();
	}
	
	return 0;
}