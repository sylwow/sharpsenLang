#include <vector>
#include <cstdio>

#include "Module.hpp"
#include "Errors.hpp"
#include "PushBackStream.hpp"
#include "Tokenizer.hpp"
#include "Compiler.hpp"

namespace sharpsenLang
{
	namespace
	{
		class File
		{
			File(const File &) = delete;
			void operator=(const File &) = delete;

		private:
			FILE *_fp;

		public:
			File(const char *path) : _fp(fopen(path, "rt"))
			{
				if (!_fp)
				{
					throw FileNotFound(std::string("'") + path + "' not found");
				}
			}

			~File()
			{
				if (_fp)
				{
					fclose(_fp);
				}
			}

			int operator()()
			{
				return fgetc(_fp);
			}
		};
	}

	class ModuleImpl
	{
	private:
		std::vector<std::pair<std::string, Function>> _externalFunctions;
		std::vector<std::string> _publicDeclarations;
		std::unordered_map<std::string, std::shared_ptr<Function>> _publicFunctions;
		std::unique_ptr<RuntimeContext> _context;

	public:
		ModuleImpl()
		{
		}

		RuntimeContext *getRuntimeContext()
		{
			return _context.get();
		}

		void addPublicFunctionDeclaration(std::string declaration, std::string name, std::shared_ptr<Function> fptr)
		{
			_publicDeclarations.push_back(std::move(declaration));
			_publicFunctions.emplace(std::move(name), std::move(fptr));
		}

		void addExternalFunctionImpl(std::string declaration, Function f)
		{
			_externalFunctions.emplace_back(std::move(declaration), std::move(f));
		}

		void load(const char *path)
		{
			File f(path);
			GetCharacter get = [&]()
			{
				return f();
			};
			PushBackStream stream(&get);

			TokensIterator it(stream);

			_context = std::make_unique<RuntimeContext>(compile(it, _externalFunctions, _publicDeclarations));

			for (const auto &p : _publicFunctions)
			{
				*p.second = _context->getPublicFunction(p.first.c_str());
			}
		}

		bool tryLoad(const char *path, std::ostream *err) noexcept
		{
			try
			{
				load(path);
				return true;
			}
			catch (const FileNotFound &e)
			{
				if (err)
				{
					*err << e.what() << std::endl;
				}
			}
			catch (const Error &e)
			{
				if (err)
				{
					File f(path);
					formatError(
						e,
						[&]()
						{
							return f();
						},
						*err);
				}
			}
			catch (const RuntimeError &e)
			{
				if (err)
				{
					*err << e.what() << std::endl;
				}
			}
			return false;
		}

		void resetGlobals()
		{
			if (_context)
			{
				_context->initialize();
			}
		}
	};

	Module::Module() : _impl(std::make_unique<ModuleImpl>())
	{
	}

	RuntimeContext *Module::getRuntimeContext()
	{
		return _impl->getRuntimeContext();
	}

	void Module::addExternalFunctionImpl(std::string declaration, Function f)
	{
		_impl->addExternalFunctionImpl(std::move(declaration), std::move(f));
	}

	void Module::addPublicFunctionDeclaration(std::string declaration, std::string name, std::shared_ptr<Function> fptr)
	{
		_impl->addPublicFunctionDeclaration(std::move(declaration), std::move(name), std::move(fptr));
	}

	void Module::load(const char *path)
	{
		_impl->load(path);
	}

	bool Module::tryLoad(const char *path, std::ostream *err) noexcept
	{
		return _impl->tryLoad(path, err);
	}

	void Module::resetGlobals()
	{
		_impl->resetGlobals();
	}

	Module::~Module()
	{
	}
}
