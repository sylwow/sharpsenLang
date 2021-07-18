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

	using Type = std::variant<SimpleType, ArrayType, FunctionType, TupleType, InitListType>;
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

	class TypeRegistry
	{
	private:
		struct TypesLess
		{
			bool operator()(const Type &t1, const Type &t2) const;
		};
		std::set<Type, TypesLess> _types;

		static Type voidType;
		static Type numberType;
		static Type stringType;

	public:
		TypeRegistry();

		TypeHandle getHandle(const Type &t);

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
