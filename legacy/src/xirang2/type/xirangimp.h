#ifndef XIRANG2_DETAIL_XIRANG2_IMP_H
#define XIRANG2_DETAIL_XIRANG2_IMP_H

#include <xirang2/type/xirang.h>
#include "typeimp.h"
#include "namespaceimp.h"

#include <xirang2/type/typebinder.h>
#include <xirang2/type/object.h>
#include <xirang2/type/typeio.h>

#include <memory>
#include <algorithm>
#include <set>
#include <map>
#include "impaccessor.h"
namespace xirang2{ namespace type{
	class XirangImp
	{
		public:
			XirangImp (const string& n, heap & al, ext_heap& eh)
				: alloc(&al), m_ext_heap(&eh), root_fs(literal("xirang2"))
			{
				name = n;
			}

			~XirangImp ()
			{
				destroy_();
			};

			CommonObject trackNew (Type t, Namespace ns, const file_path& obj_name)
			{
				XR_PRE_CONDITION(t.isMemberResolved());

                return ObjectFactory(*alloc, *m_ext_heap).create(t, ns, obj_name);
			}

			CommonObject untrackNew (Type t)
			{
				XR_PRE_CONDITION(t.isMemberResolved());

                return ObjectFactory(*alloc, *m_ext_heap).create(t);
			}

			void trackDelete (CommonObject obj, Namespace ns, const file_path& obj_name)
			{
				XR_PRE_CONDITION(obj.valid());
                XR_PRE_CONDITION(ns.findObject(obj_name).name == 0);

                ObjectDeletor(*alloc).destroy(obj, ns, obj_name);
			}

			void untrackDelete (CommonObject obj)
			{
				XR_PRE_CONDITION(obj.valid());
                ObjectDeletor(*alloc).destroy(obj);
			}

			bool removeChild(Namespace ns, const file_path& ns_name)
			{
				NamespaceImp* pImp = ImpAccessor<NamespaceImp>::getImp(ns);
				auto iter = pImp->children.find(ns_name);
				if(iter!=pImp->children.end())
				{
					destroyObj_(*iter->second);
					pImp->children.erase(iter);
					return true;
				}
				return false;
			}

			bool removeObject(Namespace ns, const file_path& obj_name)
			{
				NamespaceImp* pImp = ImpAccessor<NamespaceImp>::getImp(ns);
				auto iter = pImp->objects.find(obj_name);
				if(iter!=pImp->objects.end())
				{
					untrackDelete(iter->second);
					pImp->objects.erase(iter);
					return true;
				}
				return false;
			}

			void removeAllChildren(Namespace ns)
			{
				NamespaceImp* pImp = ImpAccessor<NamespaceImp>::getImp(ns);
				for (auto& i : pImp->children)
				{
					destroyObj_(*i.second);
				}
				pImp->children.clear();
			}

			void removeAllObjects(Namespace ns)
			{
				NamespaceImp* pImp = ImpAccessor<NamespaceImp>::getImp(ns);
				destroyObj_(*pImp);
			}

			void removeAll(Namespace ns)
			{
				removeAllChildren(ns);
				removeAllObjects(ns);
			}
            CommonObject detachObject(Namespace ns, const file_path& obj_name)
            {
                NamespaceImp* pImp = ImpAccessor<NamespaceImp>::getImp(ns);
                CommonObject ret;
				auto iter = pImp->objects.find(obj_name);
				if(iter != pImp->objects.end())
				{
                    ret = iter->second;
					pImp->objects.erase(iter);
				}
				return ret;
            }
			Type resolveType(const file_path& path, const version_type& ver, Namespace temp_root){
				BinaryTypeLoader loader(root_fs, Namespace(&root));
				return loader.load(path, ver, temp_root);
			}

			string name;
			heap* alloc;
			ext_heap* m_ext_heap;
			NamespaceImp root;
			vfs::RootFs  root_fs;
		private:

			void destroy_()
			{
				destroyTypeInternal_(root);
				destroyObj_(root);
				root.clear();
			}

			void destroyTypeInternal_(NamespaceImp& ns){
				for (auto& i : ns.children)
					destroyTypeInternal_(*i.second);

				for (auto&i : ns.types){
					i.second.current.releaseAllData();
					for (auto&j : i.second.items){
						j.second->releaseAllData();
					}
				}
			}

			void destroyObj_(NamespaceImp& ns)
			{
				for (auto& i : ns.children)
					destroyObj_(*i.second);

				for (auto& i : ns.objects)
					untrackDelete(i.second);

				ns.objects.clear();
			}
	};
}}

#endif //end XIRANG2_DETAIL_XIRANG2_IMP_H
