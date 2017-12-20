#include "xirangimp.h"
#include <xirang2/type/namespace.h>
#include <xirang2/type/type.h>
#include <xirang2/type/typealias.h>
#include <xirang2/type/object.h>

#include <cctype>

namespace xirang2{ namespace type{
	Xirang::Xirang (const string& name, heap& al, ext_heap& eh)
		: m_imp(new XirangImp(name, al, eh))
	{ }

	Xirang::~Xirang ()
	{
		check_delete(m_imp);
	}

	const string & Xirang::name () const
	{
		return m_imp->name;
	}

	CommonObject Xirang::trackNew (Type t, Namespace ns, const file_path& name)
	{
		XR_PRE_CONDITION(t.valid() && ns.valid());
		return m_imp->trackNew (t, ns, name);
	}

	CommonObject Xirang::untrackNew (Type t)
	{
		XR_PRE_CONDITION(t.valid());
		return m_imp->untrackNew (t);
	}

	void Xirang::trackDelete (CommonObject t, Namespace ns, const file_path& name)
	{
		XR_PRE_CONDITION(t.valid() && ns.valid());
		m_imp->trackDelete (t, ns, name);
	}

	void Xirang::untrackDelete (CommonObject t)
	{
		XR_PRE_CONDITION(t.valid());
		m_imp->untrackDelete (t);
	}

	bool Xirang::removeChild(Namespace ns, const file_path& name)
	{
		return m_imp->removeChild(ns,name);
	}
	bool Xirang::removeObject(Namespace ns, const file_path& name)
	{
		return m_imp->removeObject(ns, name);
	}

	void Xirang::removeAllChildren(Namespace ns)
	{
		 m_imp->removeAllChildren(ns);
	}

	void Xirang::removeAllObjects(Namespace ns)
	{
		 m_imp->removeAllObjects(ns);
	}

	void Xirang::removeAll(Namespace ns)
	{
		m_imp->removeAll(ns);
	}

    CommonObject Xirang::detachObject(Namespace ns, const file_path& name)
    {
        return m_imp->detachObject(ns, name);
    }
	Namespace Xirang::root () const
	{
		return Namespace(&m_imp->root);
	}

    heap& Xirang::get_heap() const
    {
        return *m_imp->alloc;
    }
    ext_heap& Xirang::get_ext_heap() const
    {
        return *m_imp->m_ext_heap;
    }

	vfs::RootFs& Xirang::rootFs(){
		return m_imp->root_fs;
	}
	Type Xirang::resolveType(const file_path& path, const version_type& ver, Namespace temp_root){
		return m_imp->resolveType(path, ver, temp_root);
	}

}}

