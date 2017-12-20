#ifndef XIRANG2_DETAIL_TYPE_ALIAS_IMP_H
#define XIRANG2_DETAIL_TYPE_ALIAS_IMP_H

#include <xirang2/type/xrfwd.h>
namespace xirang2{ namespace type{
	class TypeImp;
	class NamespaceImp;
	class TypeAliasImp
	{
	public:
		TypeAliasImp() : type(0), parent(0){}
		file_path name;
		file_path typeName;
		TypeImp* type;
		NamespaceImp* parent;
	};
}}

#endif //XIRANG2_DETAIL_TYPE_ALIAS_IMP_H;
