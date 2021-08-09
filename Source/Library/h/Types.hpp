#pragma once
#include <vector>
#include <variant>
#include <set>
#include <ostream>

namespace sharpsenLang
{
	enum struct SimpleType
	{
		Void,
		Number,
		String,
	};

	struct ArrayType;
	struct FunctionType;
	struct TupleType;
	struct InitListType;
	struct ClassType;

	using Type = std::variant<SimpleType, ArrayType, FunctionType, TupleType, InitListType, ClassType>;
	using TypeHandle = const Type *;

	struct ArrayType
	{
		TypeHandle innerTypeId;
	};

	struct FunctionType
	{
		struct Param
		{
			TypeHandle typeId;
			bool byRef;
		};
		TypeHandle returnTypeId;
		std::vector<Param> paramTypeId;
	};

	struct TupleType
	{
		std::vector<TypeHandle> innerTypeId;
	};

	struct InitListType
	{
		std::vector<TypeHandle> innerTypeId;
	};

	struct ClassType
	{
		std::string name;
		std::string fullName;
		std::vector<TypeHandle> properties;
		std::vector<TypeHandle> methods;
	};

	struct UndefinedInfo
	{
		std::string type;
		int lineNumber;
		int charIndex;
	};

	class TypeRegistry
	{
	private:
		struct TypesLess
		{
			bool operator()(const Type &t1, const Type &t2) const;
		};

		struct UndefinedLess
		{
			bool operator()(const UndefinedInfo &t1, const UndefinedInfo &t2) const
			{
				return t1.type < t2.type;
			}
		};
		std::set<Type, TypesLess> _types;
		std::set<UndefinedInfo, UndefinedLess> _undefinedTypes;

		static Type voidType;
		static Type numberType;
		static Type stringType;

	public:
		TypeRegistry();

		TypeHandle getHandle(const Type &t);

		TypeHandle getRegisteredClassHandle(const std::string &name) const;

		TypeHandle checkClassRegistration(const std::string &name, int lineNumber, int charIndex);

		std::vector<UndefinedInfo> getUndefinedTypes() const;

		static TypeHandle getVoidHandle()
		{
			return &voidType;
		}

		static TypeHandle getNumberHandle()
		{
			return &numberType;
		}

		static TypeHandle getStringHandle()
		{
			return &stringType;
		}
	};
}

namespace std
{
	std::string to_string(sharpsenLang::TypeHandle t);
}
