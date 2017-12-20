#ifndef XIRANG2_XIRANG2_H__
#define XIRANG2_XIRANG2_H__

#include <xirang2/type/xrfwd.h>
#include <xirang2/type/type.h>
#include <xirang2/type/namespace.h>
#include <xirang2/type/typealias.h>
#include <xirang2/vfs.h>

namespace xirang2{ namespace type{
	class XirangImp;

	class XR_API Xirang
	{
	public:

		Xirang (const string & name, heap & al, ext_heap& eh);
		~Xirang ();

		const string & name () const;

        //deprecated
		CommonObject trackNew (Type t, Namespace ns, const file_path& name);
        //deprecated
		CommonObject untrackNew (Type t);
        //deprecated
		void trackDelete (CommonObject t, Namespace ns, const file_path& name);
        //deprecated
		void untrackDelete (CommonObject t);

		Namespace root () const;

        heap& get_heap() const;
        ext_heap& get_ext_heap() const;

		/// remove the sub namespace with given name.
		/// return true if remove successfully
		bool removeChild(Namespace ns, const file_path& name);
		/// remove the sub object with given name.
		/// return true if remove successfully
		bool removeObject(Namespace ns, const file_path& name);

		/// remove all sub namespaces.
		void removeAllChildren(Namespace ns);

		/// remove all sub objects.
		void removeAllObjects(Namespace ns);

		/// remove all sub objects and namespaces.
		void removeAll(Namespace ns);

        CommonObject detachObject(Namespace ns, const file_path& name);

		vfs::RootFs& rootFs();

		/// load type, and put it under namespace temp_root, temp_root will reference root() namespace.
		/// \pre temp_root.valid()
		/// \note you can pass root() as temp_root, but in this case, there is no strong exception safety guarantee.
		Type resolveType(const file_path& path, const version_type& ver, Namespace temp_root);
	private:
		//non-copyable
		Xirang (Xirang &);
		void operator= (Xirang &);

		XirangImp *m_imp;
	};

	XR_API void SetupXirang(Xirang& xr);
}}
#endif				//end XIRANG2_XIRANG2_H__
