#include <sstream>

#include "errors.hpp"

namespace sharpsenLang
{
	Error::Error(std::string message, size_t lineNumber, size_t charIndex) noexcept
		: _message(std::move(message)),
		  _lineNumber(lineNumber),
		  _charIndex(charIndex)
	{
	}

	const char *Error::what() const noexcept
	{
		return _message.c_str();
	}

	size_t Error::lineNumber() const noexcept
	{
		return _lineNumber;
	}

	size_t Error::charIndex() const noexcept
	{
		return _charIndex;
	}

	Error parsingError(std::string_view message, size_t lineNumber, size_t charIndex)
	{
		std::string error_message("Parsing error: ");
		error_message += message;
		return Error(std::move(error_message), lineNumber, charIndex);
	}

	Error syntaxError(std::string_view message, size_t lineNumber, size_t charIndex)
	{
		std::string error_message("Syntax error: ");
		error_message += message;
		return Error(std::move(error_message), lineNumber, charIndex);
	}

	Error semanticError(std::string_view message, size_t lineNumber, size_t charIndex)
	{
		std::string error_message("Semantic error: ");
		error_message += message;
		return Error(std::move(error_message), lineNumber, charIndex);
	}

	Error compilerError(std::string_view message, size_t lineNumber, size_t charIndex)
	{
		std::string error_message("Compiler error: ");
		error_message += message;
		return Error(std::move(error_message), lineNumber, charIndex);
	}

	Error unexpectedError(std::string_view unexpected, size_t lineNumber, size_t charIndex)
	{
		std::string message("Unexpected '");
		message += unexpected;
		message += "'";
		return parsingError(message, lineNumber, charIndex);
	}

	Error unexpectedSyntaxError(std::string_view unexpected, size_t lineNumber, size_t charIndex)
	{
		std::string message("Unexpected '");
		message += unexpected;
		message += "'";
		return syntaxError(message, lineNumber, charIndex);
	}

	Error expectedSyntaxError(std::string_view expected, size_t lineNumber, size_t charIndex)
	{
		std::string message("Expected '");
		message += expected;
		message += "'";
		return syntaxError(message, lineNumber, charIndex);
	}

	Error undeclaredError(std::string_view undeclared, size_t lineNumber, size_t charIndex)
	{
		std::string message("Undeclared identifier '");
		message += undeclared;
		message += "'";
		return semanticError(message, lineNumber, charIndex);
	}

	Error wrongTypeError(std::string_view source, std::string_view destination,
						 bool lvalue, size_t lineNumber,
						 size_t charIndex)
	{
		std::string message;
		if (lvalue)
		{
			message += "'";
			message += source;
			message += "' is not a lvalue";
		}
		else
		{
			message += "Cannot convert '";
			message += source;
			message += "' to '";
			message += destination;
			message += "'";
		}
		return semanticError(message, lineNumber, charIndex);
	}

	Error alreadyDeclaredError(std::string_view name, size_t lineNumber, size_t charIndex)
	{
		std::string message = "'";
		message += name;
		message += "' is already declared";
		return semanticError(message, lineNumber, charIndex);
	}

	void formatError(const Error &err, const GetCharacter &source, std::ostream &output)
	{
		output << "(" << (err.lineNumber() + 1) << ") " << err.what() << std::endl;

		size_t charIndex = 0;

		for (size_t lineNumber = 0; lineNumber < err.lineNumber(); ++charIndex)
		{
			int c = source();
			if (c < 0)
			{
				return;
			}
			else if (c == '\n')
			{
				++lineNumber;
			}
		}

		size_t index_in_line = err.charIndex() - charIndex;

		std::string line;
		for (size_t idx = 0;; ++idx)
		{
			int c = source();
			if (c < 0 || c == '\n' || c == '\r')
			{
				break;
			}
			line += char(c == '\t' ? ' ' : c);
		}

		output << line << std::endl;

		for (size_t idx = 0; idx < index_in_line; ++idx)
		{
			output << " ";
		}

		output << "^" << std::endl;
	}

	RuntimeError::RuntimeError(std::string message) noexcept : _message(std::move(message))
	{
	}

	const char *RuntimeError::what() const noexcept
	{
		return _message.c_str();
	}

	void runtimeAssertion(bool b, const char *message)
	{
		if (!b)
		{
			throw RuntimeError(message);
		}
	}

	FileNotFound::FileNotFound(std::string message) noexcept : _message(std::move(message))
	{
	}

	const char *FileNotFound::what() const noexcept
	{
		return _message.c_str();
	}
}
