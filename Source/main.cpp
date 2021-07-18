#include <iostream>
#include "Module.hpp"
#include "StandardFunctions.hpp"

int main() {
	std::string path = __FILE__;
	path = path.substr(0, path.find_last_of("/\\") + 1) + "test.stk";
	
	using namespace sharpsenLang;
	
	Module m;
	
	addStandardFunctions(m);
	
	m.addExternalFunction("greater", std::function<Number(Number, Number)>([](Number x, Number y){
		return x > y;
	}));
	
	auto s_main = m.createPublicFunctionCaller<void>("main");
	
	if (m.tryLoad(path.c_str(), &std::cerr)) {
		s_main();
	}
	
	return 0;
}