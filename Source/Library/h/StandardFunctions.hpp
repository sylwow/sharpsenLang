#pragma once


namespace sharpsenLang {

	class Module;
	
	void add_math_functions(Module& m);
	void add_string_functions(Module& m);
	void add_trace_functions(Module& m);
	
	void add_standard_functions(Module& m);
}

