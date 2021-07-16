#pragma once

#include <exception>
#include <functional>
#include <string>
#include <string_view>
#include <ostream>

namespace sharpsenLang
{
	class Error : public std::exception
	{
	private:
		std::string _message;
		size_t _lineNumber;
		size_t _charIndex;

	public:
		Error(std::string message, size_t lineNumber, size_t charIndex) noexcept;

		const char *what() const noexcept override;
		size_t lineNumber() const noexcept;
		size_t charIndex() const noexcept;
	};

	Error parsingError(std::string_view message, size_t lineNumber, size_t charIndex);
	Error syntaxError(std::string_view message, size_t lineNumber, size_t charIndex);
	Error semanticError(std::string_view message, size_t lineNumber, size_t charIndex);
	Error compilerError(std::string_view message, size_t lineNumber, size_t charIndex);

	Error unexpectedError(std::string_view unexpected, size_t lineNumber, size_t charIndex);
	Error unexpectedSyntaxError(std::string_view unexpected, size_t lineNumber, size_t charIndex);
	Error expectedSyntaxError(std::string_view expected, size_t lineNumber, size_t charIndex);
	Error undeclaredError(std::string_view undeclared, size_t lineNumber, size_t charIndex);
	Error wrongTypeError(std::string_view source, std::string_view destination, bool lvalue,
						 size_t lineNumber, size_t charIndex);
	Error alreadyDeclaredError(std::string_view name, size_t lineNumber, size_t charIndex);

	using GetCharacter = std::function<int()>;
	void formatError(const Error &err, const GetCharacter &source, std::ostream &output);

	class RuntimeError : public std::exception
	{
	private:
		std::string _message;

	public:
		RuntimeError(std::string message) noexcept;

		const char *what() const noexcept override;
	};

	void runtimeAssertion(bool b, const char *message);

	class FileNotFound : public std::exception
	{
	private:
		std::string _message;

	public:
		FileNotFound(std::string message) noexcept;

		const char *what() const noexcept override;
	};
};
