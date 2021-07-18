#include <vector>
#include <cstdio>

#include "Module.hpp"
#include "Errors.hpp"
#include "PushBackStream.hpp"
#include "Tokenizer.hpp"
#include "Compiler.hpp"

namespace sharpsenLang {
	namespace {
		class file{
			file(const file&) = delete;
			void operator=(const file&) = delete;
		private:
			FILE* _fp;
		public:
			file(const char* path):
				_fp(fopen(path, "rt"))
			{
				if (!_fp) {
					throw FileNotFound(std::string("'") + path + "' not found");
				}
			}
			
			~file() {
				if (_fp) {
					fclose(_fp);
				}
			}
			
			int operator()() {
				return fgetc(_fp);
			}
		};
	}

	class module_impl {
	private:
		std::vector<std::pair<std::string, function> > _external_functions;
		std::vector<std::string> _public_declarations;
		std::unordered_map<std::string, std::shared_ptr<function> > _public_functions;
		std::unique_ptr<RuntimeContext> _context;
	public:
		module_impl(){
		}
		
		RuntimeContext* get_runtime_context() {
			return _context.get();
		}
		
		void add_public_function_declaration(std::string declaration, std::string name, std::shared_ptr<function> fptr) {
			_public_declarations.push_back(std::move(declaration));
			_public_functions.emplace(std::move(name), std::move(fptr));
		}
		
		void add_external_function_impl(std::string declaration, function f) {
			_external_functions.emplace_back(std::move(declaration), std::move(f));
		}
		
		void load(const char* path) {
			file f(path);
			GetCharacter get = [&](){
				return f();
			};
			PushBackStream stream(&get);
			
			TokensIterator it(stream);
			
			_context = std::make_unique<RuntimeContext>(compile(it, _external_functions, _public_declarations));
			
			for (const auto& p : _public_functions) {
				*p.second = _context->getPublicFunction(p.first.c_str());
			}
		}
		
		bool try_load(const char* path, std::ostream* err) noexcept{
			try {
				load(path);
				return true;
			} catch(const FileNotFound& e) {
				if (err) {
					*err << e.what() << std::endl;
				}
			} catch(const Error& e) {
				if (err) {
					file f(path);
					formatError(
						e,
						[&](){
							return f();
						},
						*err
					);
				}
			} catch(const RuntimeError& e) {
				if (err) {
					*err << e.what() << std::endl;
				}
			}
			return false;
		}
		
		void reset_globals() {
			if (_context) {
				_context->initialize();
			}
		}
	};
	
	Module::Module():
		_impl(std::make_unique<module_impl>())
	{
	}
	
	RuntimeContext* Module::get_runtime_context() {
		return _impl->get_runtime_context();
	}
	
	void Module::add_external_function_impl(std::string declaration, function f) {
		_impl->add_external_function_impl(std::move(declaration), std::move(f));
	}

	void Module::add_public_function_declaration(std::string declaration, std::string name, std::shared_ptr<function> fptr) {
		_impl->add_public_function_declaration(std::move(declaration), std::move(name), std::move(fptr));
	}
	
	void Module::load(const char* path) {
		_impl->load(path);
	}
	
	bool Module::try_load(const char* path, std::ostream* err) noexcept{
		return _impl->try_load(path, err);
	}
	
	void Module::reset_globals() {
		_impl->reset_globals();
	}
	
	Module::~Module() {
	}
}
