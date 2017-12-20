#ifndef XIRANG2_DETAIL_NAMESPACE_IMP_H
#define XIRANG2_DETAIL_NAMESPACE_IMP_H

#include "typeimp.h"
#include "typealiasimp.h"
#include <xirang2/type/object.h>
#include <xirang2/type/type.h>
#include <xirang2/type/namespace.h>
#include <xirang2/type/typealias.h>

#include <map>

namespace xirang2{ namespace type{

	class TypeImp;
	struct TypeSynonymImp
	{
		Type current;
		std::map<version_type, unique_ptr<TypeImp> > items;
	};

	class NamespaceImp
	{
		public:
			NamespaceImp() : parent(0){}
			~NamespaceImp()
			{
				clear();
			}

			void clear()
			{
				XR_PRE_CONDITION(objects.empty());

				decltype(alias)().swap(alias);
				decltype(types)().swap(types);
				decltype(children)().swap(children);

				parent = 0;
			}

			file_path name;

			std::map<file_path, TypeSynonymImp > types;
			std::map<file_path, unique_ptr<NamespaceImp> > children;
			std::map<file_path, unique_ptr<TypeAliasImp> > alias;
			std::map<file_path, CommonObject> objects;

			NamespaceImp *parent;

	};

	struct TypeSynonymIterator
	{
		typedef std::map<file_path, TypeSynonymImp>::iterator iterator;
		TypeSynonymIterator(iterator itr) : pos(itr){}

		const TypeSynonym& operator*() const { cache = TypeSynonym(&pos->second); return cache;}
		const TypeSynonym* operator->() const { return &**this;}
		void operator++ () { ++pos;}
		void operator-- () { --pos;}
		bool operator== (const TypeSynonymIterator& rhs) const { return pos == rhs.pos;}

		mutable TypeSynonym cache;
		iterator pos;
	};

	struct TypeIterator
	{
		typedef std::map<version_type, unique_ptr<TypeImp> >::iterator RealIterator;
		TypeIterator(RealIterator itr) : rpos(itr){}

		const Type& operator*() const { cache = Type(rpos->second.get()); return cache;}
        const Type* operator->() const { return &**this;}

		void operator++ () { ++rpos;}
		void operator-- () { --rpos;}
		bool operator== (const TypeIterator& rhs) const { return rpos == rhs.rpos;}

		RealIterator rpos;
		mutable Type cache;
	};

	struct TypeCurrentIterator
	{
		typedef std::map<file_path, TypeSynonymImp>::iterator RealIterator;
		TypeCurrentIterator(RealIterator itr) : rpos(itr){}

		const Type& operator*() const {
			return rpos->second.current;
		}
        const Type* operator->() const { return &**this;}

		void operator++ () { ++rpos;}
		void operator-- () { --rpos;}
		bool operator== (const TypeCurrentIterator& rhs) const { return rpos == rhs.rpos;}

		RealIterator rpos;
	};

	struct TypeAliasIteratorImp
	{
		typedef std::map < file_path, unique_ptr<TypeAliasImp> >::iterator RealIterator;
		TypeAliasIteratorImp(const RealIterator& itr) : rpos(itr){}

		const TypeAlias& operator*() const { cache = TypeAlias(rpos->second.get());return cache;}
        const TypeAlias* operator->() const { return &**this;}
		void operator++ () { ++rpos;}
		void operator-- () { --rpos;}
		bool operator== (const TypeAliasIteratorImp& rhs) const { return rpos == rhs.rpos;}

		RealIterator rpos;
		mutable TypeAlias cache;
	};

	struct NamespaceIteratorImp
	{
		typedef std::map < file_path, unique_ptr<NamespaceImp> >::iterator RealIterator;
		NamespaceIteratorImp(const RealIterator& itr) : rpos(itr){}

		const Namespace& operator*() const { cache = Namespace(rpos->second.get()); return cache;}
        const Namespace* operator->() const { return &**this;}
		void operator++ () { ++rpos;}
		void operator-- () { --rpos;}
		bool operator== (const NamespaceIteratorImp& rhs) const { return rpos == rhs.rpos;}

		RealIterator rpos;
		mutable Namespace cache;
	};

	struct ObjIteratorImp
	{
		typedef	std::map < file_path, CommonObject >::iterator RealIterator;

		ObjIteratorImp(const RealIterator& itr) : rpos(itr)
        {
            value.name = 0;
        }

        const NameValuePair& operator*() const {
            value.name = &rpos->first;
            value.value = rpos->second;
            return value;
        }

        const NameValuePair* operator->() const {
            value.name = &rpos->first;
            value.value = rpos->second;
            return &value;
        }

		void operator++ () { ++rpos;}
		void operator-- () { --rpos;}
		bool operator== (const ObjIteratorImp& rhs) const { return rpos == rhs.rpos;}

		RealIterator rpos;
        mutable NameValuePair value;
	};

}}

#endif				//end XIRANG2_DETAIL_NAMESPACE_IMP_H

