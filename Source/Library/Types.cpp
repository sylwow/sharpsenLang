#include "Types.hpp"
#include "Helpers.hpp"

namespace sharpsenLang
{
	bool TypeRegistry::TypesLess::operator()(const Type &t1, const Type &t2) const
	{
		const size_t idx1 = t1.index();
		const size_t idx2 = t2.index();

		if (idx1 != idx2)
		{
			return idx1 < idx2;
		}

		switch (idx1)
		{
		case 0:
			return std::get<0>(t1) < std::get<0>(t2);
		case 1:
			return std::get<1>(t1).innerTypeId < std::get<1>(t2).innerTypeId;
		case 2:
		{
			const FunctionType &ft1 = std::get<2>(t1);
			const FunctionType &ft2 = std::get<2>(t2);

			if (ft1.returnTypeId != ft2.returnTypeId)
			{
				return ft1.returnTypeId < ft2.returnTypeId;
			}

			if (ft1.paramTypeId.size() != ft2.paramTypeId.size())
			{
				return ft1.paramTypeId.size() < ft2.paramTypeId.size();
			}

			for (size_t i = 0; i < ft1.paramTypeId.size(); ++i)
			{
				if (ft1.paramTypeId[i].typeId != ft2.paramTypeId[i].typeId)
				{
					return ft1.paramTypeId[i].typeId < ft2.paramTypeId[i].typeId;
				}
				if (ft1.paramTypeId[i].byRef != ft2.paramTypeId[i].byRef)
				{
					return ft2.paramTypeId[i].byRef;
				}
			}
			return false;
		}
		case 3:
		{
			const TupleType &tt1 = std::get<3>(t1);
			const TupleType &tt2 = std::get<3>(t2);

			if (tt1.innerTypeId.size() != tt2.innerTypeId.size())
			{
				return tt1.innerTypeId.size() < tt2.innerTypeId.size();
			}

			for (size_t i = 0; i < tt1.innerTypeId.size(); ++i)
			{
				if (tt1.innerTypeId[i] != tt2.innerTypeId[i])
				{
					return tt1.innerTypeId[i] < tt2.innerTypeId[i];
				}
			}
			return false;
		}
		case 4:
		{
			const InitListType &ilt1 = std::get<4>(t1);
			const InitListType &ilt2 = std::get<4>(t2);

			if (ilt1.innerTypeId.size() != ilt2.innerTypeId.size())
			{
				return ilt1.innerTypeId.size() < ilt2.innerTypeId.size();
			}

			for (size_t i = 0; i < ilt1.innerTypeId.size(); ++i)
			{
				if (ilt1.innerTypeId[i] != ilt2.innerTypeId[i])
				{
					return ilt1.innerTypeId[i] < ilt2.innerTypeId[i];
				}
			}
			return false;
		}
		case 5:
			return std::get<5>(t1).fullName < std::get<5>(t2).fullName;
		}

		return false;
	}

	TypeRegistry::TypeRegistry()
	{
	}

	bool TypeRegistry::isRegistered(const Type &t)
	{
		return _types.contains(t);
	}

	TypeHandle TypeRegistry::getHandle(const Type &t)
	{
		return std::visit(
			overloaded{
				[](SimpleType t)
				{
					switch (t)
					{
					case SimpleType::Void:
						return TypeRegistry::getVoidHandle();
					case SimpleType::Number:
						return TypeRegistry::getNumberHandle();
					case SimpleType::String:
						return TypeRegistry::getStringHandle();
					}
				},
				[this](const auto &t)
				{
					return &(*(_types.insert(t).first));
				}},
			t);
	}

	Type TypeRegistry::voidType = SimpleType::Void;
	Type TypeRegistry::numberType = SimpleType::Number;
	Type TypeRegistry::stringType = SimpleType::String;
}

namespace std
{
	using namespace sharpsenLang;
	std::string to_string(TypeHandle t)
	{
		return std::visit(
			overloaded{
				[](SimpleType st)
				{
					switch (st)
					{
					case SimpleType::Void:
						return std::string("void");
					case SimpleType::Number:
						return std::string("number");
					case SimpleType::String:
						return std::string("string");
					}
				},
				[](const ArrayType &at)
				{
					std::string ret = to_string(at.innerTypeId);
					ret += "[]";
					return ret;
				},
				[](const FunctionType &ft)
				{
					std::string ret = to_string(ft.returnTypeId) + "(";
					const char *separator = "";
					for (const FunctionType::Param &p : ft.paramTypeId)
					{
						ret += separator + to_string(p.typeId) + (p.byRef ? "&" : "");
						separator = ",";
					}
					ret += ")";
					return ret;
				},
				[](const TupleType &tt)
				{
					std::string ret = "[";
					const char *separator = "";
					for (TypeHandle it : tt.innerTypeId)
					{
						ret += separator + to_string(it);
						separator = ",";
					}
					ret += "]";
					return ret;
				},
				[](const InitListType &ilt)
				{
					std::string ret = "{";
					const char *separator = "";
					for (TypeHandle it : ilt.innerTypeId)
					{
						ret += separator + to_string(it);
						separator = ",";
					}
					ret += "}";
					return ret;
				},
				[](const ClassType &ct)
				{
					return ct.fullName;
				}},
			*t);
	}
}
