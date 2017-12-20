#include <xirang2/type/object.h>
#include <xirang2/type/itypebinder.h>
#include "typeimp.h"
#include <xirang2/type/namespace.h>
#include <xirang2/type/xirang.h>
#include "namespaceimp.h"
#include "impaccessor.h"

namespace xirang2{ namespace type{

        UninitObjectPtr::UninitObjectPtr(Type t, heap& al)
            : m_type(t), m_al(al), m_data(0), m_dtor_enabled(false)
        {
            XR_PRE_CONDITION(t.valid());
			XR_PRE_CONDITION(t.isMemberResolved());
            reset();
        }
        UninitObjectPtr::~UninitObjectPtr()
        {
            if (m_data)
            {
                if (m_dtor_enabled)
                    m_type.methods().destruct(CommonObject(m_type, m_data));
                m_al.free(m_data, m_type.payload(), m_type.align());
            }
        }
        void* UninitObjectPtr::release()
        {
            void* ret = m_data;
            m_data = 0;
            m_dtor_enabled = false;
            return ret;
        }

        void* UninitObjectPtr::get() const
        {
            return m_data;
        }

        void UninitObjectPtr::reset()
        {
            XR_PRE_CONDITION(m_data == 0 && !m_dtor_enabled);

            m_data  = m_al.malloc(m_type.payload(), m_type.align(), 0);
        }

        bool UninitObjectPtr::dtorEnabled() const
        {
            return m_dtor_enabled;
        }

        void UninitObjectPtr::enableDtor()
        {
            XR_PRE_CONDITION(m_data && !m_dtor_enabled);
            m_dtor_enabled = true;
        }
	// *****************************
	// ConstCommonObject
	// *****************************
	ConstCommonObject::ConstCommonObject (Type t, const void* p) : m_type (t), m_obj(const_cast<void*>(p))
	{
		XR_PRE_CONDITION ((p == 0 && !t.valid()) || (p != 0 && t.valid() ) );
	}

	bool ConstCommonObject::valid () const
	{
		return m_obj != 0;
	}
	ConstCommonObject::operator bool () const
	{
		return m_obj != 0;
	}

	Type ConstCommonObject::type () const
	{
		return m_type;
	}

	ConstSubObjRange ConstCommonObject::members() const
	{
		XR_PRE_CONDITION (valid ());
        ConstSubObjIterator first(ConstSubObject (m_type, m_obj, 0));
        ConstSubObjIterator last(ConstSubObject (m_type, m_obj, m_type.memberCount()));

        return ConstSubObjRange(
            ConstSubObjRange::iterator(first),
            ConstSubObjRange::iterator(last)
            );
	}

	ConstSubObject  ConstCommonObject::asSubObject () const
	{
		XR_PRE_CONDITION (valid ());

		return ConstSubObject(m_type, m_obj, ConstSubObject::PhonyIndex);
	}

    ConstSubObject ConstCommonObject::getMember(size_t idx) const
    {
        XR_PRE_CONDITION (valid ());
        return ConstSubObject(m_type, m_obj, idx);
    }

    ConstSubObject ConstCommonObject::getMember(const string& name) const
    {
        XR_PRE_CONDITION (valid ());
        TypeItem ti = m_type.member(name);
        return ti.valid() ? ConstSubObject(m_type, m_obj, ti.index()) : ConstSubObject();
    }

    const void * ConstCommonObject::data() const
	{
		XR_PRE_CONDITION (valid ());

		return m_obj;
	}

	int ConstCommonObject::compare (const ConstCommonObject & rhs) const
	{
		return comparePtr(m_obj, rhs.m_obj);
	}

	// *****************************
	// CommonObject
	// *****************************
	CommonObject::CommonObject (Type t, void* p) : ConstCommonObject(t, p)
	{
		XR_PRE_CONDITION ((p == 0 && !t.valid()) || (p != 0 && t.valid() ) );
	}

	SubObjRange CommonObject::members() const
	{
		XR_PRE_CONDITION (valid ());
        SubObjIterator first(SubObject (m_type, m_obj, 0));
        SubObjIterator last(SubObject (m_type, m_obj, m_type.memberCount()));
		return SubObjRange(
            SubObjRange::iterator(first),
			SubObjRange::iterator(last)
				);
	}

	SubObject  CommonObject::asSubObject () const
	{
		XR_PRE_CONDITION (valid ());

		return SubObject(m_type, m_obj, SubObject::PhonyIndex);
	}

    SubObject CommonObject::getMember(size_t idx) const
    {
        XR_PRE_CONDITION (valid ());
        return SubObject(m_type, m_obj, idx);
    }

    SubObject CommonObject::getMember(const string& name) const
    {
        XR_PRE_CONDITION (valid ());
        TypeItem ti = m_type.member(name);
        return ti.valid() ? SubObject(m_type, m_obj, ti.index()) : SubObject();
    }

	void * CommonObject::data() const
	{
		return m_obj;
	}

	void  CommonObject::assign(ConstCommonObject obj)
	{
		XR_PRE_CONDITION (valid ());
		type().methods().assign(obj, *this);
	}

	// *****************************
	// ConstSubObject
	// *****************************
	ConstSubObject::ConstSubObject (Type t, const void *p, std::size_t idx)
		: m_type (t), m_obj(const_cast<void*>(p)), m_index (idx)
	{
		XR_PRE_CONDITION ((p == 0 && !t.valid() && idx == 0)
            || (p != 0 && t.valid() && t.isComplete()
					&& (idx <= t.memberCount() || idx == PhonyIndex)));
	}

	bool ConstSubObject::valid () const
	{
		return m_obj != 0;
	}

	bool ConstSubObject::isPhony_ () const
	{
		return m_index == PhonyIndex;
	}

	bool ConstSubObject::regular_ () const
	{
		return m_obj != 0 && (m_index < m_type.memberCount() || isPhony_());
	}

	Type ConstSubObject::type () const
	{
		XR_PRE_CONDITION (regular_ ());
		return isPhony_() ? m_type : m_type.member(m_index).type();
	}

    Type ConstSubObject::ownerType () const
	{
		return m_type;
	}

    TypeItem ConstSubObject::itemInfo() const{
        XR_PRE_CONDITION (regular_ ());
        return isPhony_() ? TypeItem() : m_type.member(m_index);
    }
	const void * ConstSubObject::data() const
	{
		XR_PRE_CONDITION (regular_());

		return isPhony_()
			? m_obj
			: m_index < m_type.memberCount()
            ? (reinterpret_cast<char*>(m_obj) + m_type.member(m_index).offset())
            : (reinterpret_cast<char*>(m_obj) + m_type.payload());
	}

    const void * ConstSubObject::ownerData() const
    {
        return m_obj;
    }

	ConstCommonObject ConstSubObject::asCommonObject() const
	{
		return ConstCommonObject(type(), data());
	}

	int ConstSubObject::compare (const ConstSubObject & rhs) const
	{
		int res = comparePtr(m_obj, rhs.m_obj);
		return res == 0 ? int(m_index - rhs.m_index) : res;
	}

    size_t ConstSubObject::index() const
    {
        return m_index;
    }
	// *****************************
	// SubObject
	// *****************************
	SubObject::SubObject (Type t, void *p, std::size_t idx)
		: ConstSubObject(t, p, idx)
	{
		XR_PRE_CONDITION ((p == 0 && !t.valid() && idx == 0)
            || (p != 0 && t.valid() && t.isComplete()
					&& (idx <= t.memberCount() || idx == PhonyIndex)));
	}

	CommonObject SubObject::asCommonObject() const
	{
		return CommonObject(type(), data());
	}

	void * SubObject::data() const
	{
		XR_PRE_CONDITION (valid ());

		return isPhony_()
			? m_obj
			: (reinterpret_cast<char*>(m_obj) + m_type.member(m_index).offset());
	}

	// *****************************
	// ConstSubObjIterator
	// *****************************
	ConstSubObjIterator::ConstSubObjIterator () : m_subObj ()
	{
	}

	ConstSubObjIterator::ConstSubObjIterator (const ConstSubObject & rhs)
        :m_subObj (rhs.ownerType(), const_cast<void*>(rhs.ownerData()), rhs.index())
	{
	}

	const ConstSubObject& ConstSubObjIterator::operator* () const
	{
		XR_PRE_CONDITION (m_subObj.valid ());
		return m_subObj;
	}


    const ConstSubObject* ConstSubObjIterator::operator->() const
    {
        return const_cast<SubObject*>(&m_subObj);
    }

	ConstSubObjIterator & ConstSubObjIterator::operator++ ()
	{
		XR_PRE_CONDITION (m_subObj.valid ());
		++m_subObj.m_index;
		return *this;
	}
	ConstSubObjIterator ConstSubObjIterator::operator++ (int)
	{
		XR_PRE_CONDITION (m_subObj.valid ());
		ConstSubObjIterator tmp = *this;
		++(*this);
		return tmp;
	}
	ConstSubObjIterator & ConstSubObjIterator::operator-- ()
	{
		XR_PRE_CONDITION (m_subObj.valid ());
		--m_subObj.m_index;
		return *this;
	}
	ConstSubObjIterator ConstSubObjIterator::operator-- (int)
	{
		XR_PRE_CONDITION (m_subObj.valid ());
		ConstSubObjIterator tmp = *this;
		--(*this);
		return tmp;
	}

	bool ConstSubObjIterator::equals (const ConstSubObjIterator & rhs) const
	{
		return m_subObj.compare(rhs.m_subObj) == 0;
	}

	// *****************************
	// SubObjIterator
	// *****************************
	SubObjIterator::SubObjIterator ()
	{
	}

	SubObjIterator::SubObjIterator (const SubObject & rhs):ConstSubObjIterator (rhs)
	{
	}

	const SubObject& SubObjIterator::operator* () const
	{
		XR_PRE_CONDITION (m_subObj.valid ());
		return m_subObj;
	}

    const SubObject* SubObjIterator::operator->() const
	{
		XR_PRE_CONDITION (m_subObj.valid ());
		return &m_subObj;
	}

	SubObjIterator & SubObjIterator::operator++ ()
	{
		XR_PRE_CONDITION (m_subObj.valid ());
		++m_subObj.m_index;
		return *this;
	}
	SubObjIterator SubObjIterator::operator++ (int)
	{
		XR_PRE_CONDITION (m_subObj.valid ());
		SubObjIterator tmp = *this;
		++(*this);
		return tmp;
	}
	SubObjIterator & SubObjIterator::operator-- ()
	{
		XR_PRE_CONDITION (m_subObj.valid ());
		--m_subObj.m_index;
		return *this;
	}
	SubObjIterator SubObjIterator::operator-- (int)
	{
		XR_PRE_CONDITION (m_subObj.valid ());
		SubObjIterator tmp = *this;
		--(*this);
		return tmp;
	}

    ObjectFactory::ObjectFactory (const Xirang& xi)
        : m_alloc(&xi.get_heap()), m_ext_alloc(&xi.get_ext_heap())
	{}

    ObjectFactory::ObjectFactory (heap & al, ext_heap& eh)
		: m_alloc(&al), m_ext_alloc(&eh)
	{}

	CommonObject ObjectFactory::create(Type t)
	{
		XR_PRE_CONDITION(t.valid());

        UninitObjectPtr ptr(t, *m_alloc);
        CommonObject ret(t, ptr.get());
        t.methods().construct(ret, *m_alloc, *m_ext_alloc);

        ptr.release();
        return ret;
	}

	CommonObject ObjectFactory::create(Type t, Namespace ns, const file_path& name)
	{
		XR_PRE_CONDITION(t.valid());
		XR_PRE_CONDITION(ns.valid());
        XR_PRE_CONDITION(!name.empty());
        XR_PRE_CONDITION(ns.findObject(name).name == 0);

        UninitObjectPtr ptr(t, *m_alloc);
        CommonObject ret(t, ptr.get());

        t.methods().construct(ret, *m_alloc, *m_ext_alloc);
        ptr.enableDtor();

        ImpAccessor<NamespaceImp>::getImp(ns)->objects[name] = ret;
        ptr.release();
        return ret;


	}

	CommonObject ObjectFactory::clone(ConstCommonObject obj)
	{
		XR_PRE_CONDITION(obj.valid());
        Type t = obj.type();

        UninitObjectPtr ptr(t, *m_alloc);
        CommonObject ret(t, ptr.get());

        t.methods().construct(ret, *m_alloc, *m_ext_alloc);
        ptr.enableDtor();
        t.methods().assign(obj, ret);
        ptr.release();
        return ret;
	}

	CommonObject ObjectFactory::clone(ConstCommonObject obj, Namespace ns, const file_path& name)
	{
		XR_PRE_CONDITION(ns.valid());
        XR_PRE_CONDITION(obj.valid());
        XR_PRE_CONDITION(ns.findObject(name).name == 0);

        Type t = obj.type();

        UninitObjectPtr ptr(t, *m_alloc);
        CommonObject ret(t, ptr.get());

        t.methods().construct(ret, *m_alloc, *m_ext_alloc);
        ptr.enableDtor();
        t.methods().assign(obj, ret);
        ImpAccessor<NamespaceImp>::getImp(ns)->objects[name] = ret;

        ptr.release();
        return ret;
	}

    heap& ObjectFactory::getHeap() const { return *m_alloc;}

    ext_heap& ObjectFactory::getExtHeap() const { return *m_ext_alloc;}

    ObjectDeletor::ObjectDeletor(heap & al)
        : m_alloc(&al)
    {
    }

    void ObjectDeletor::destroy(CommonObject obj) const
    {
        XR_PRE_CONDITION(obj.valid());
        Type t = obj.type();
        if (!t.isPod())
            t.methods().destruct(obj);
        m_alloc->free(obj.data(), t.payload(), t.align());
    }

    void ObjectDeletor::operator()(CommonObject obj) const
    {
        destroy(obj);
    }

    void ObjectDeletor::destroy(CommonObject obj,  Namespace ns, const file_path& name) const
    {
        XR_PRE_CONDITION(obj.valid());
        XR_PRE_CONDITION(ns.findObject(name).value == obj);
        ImpAccessor<NamespaceImp>::getImp(ns)->objects.erase(name);
        destroy(obj);

    }

    void ObjectDeletor::operator()(CommonObject obj,  Namespace ns, const file_path& name) const
    {
        destroy(obj, ns, name);
    }

    heap& ObjectDeletor::getHeap() const
    {
        return *m_alloc;
    }
    ScopedObjectCreator::ScopedObjectCreator(ScopedObjectCreator&&  rhs)
        : m_type(rhs.m_type), m_al(rhs.m_al)
        , m_ext_al(rhs.m_ext_al), m_data(rhs.m_data)
    {
        rhs.m_data = 0;
    }

    /// move assignment
    ScopedObjectCreator& ScopedObjectCreator::operator=(ScopedObjectCreator&&  rhs)
    {
        ScopedObjectCreator(std::move(rhs)).swap(*this);
        return *this;
    }

    ///ctor
    ScopedObjectCreator::ScopedObjectCreator(Type t, const Xirang& xi)
        : m_type(t), m_al(&xi.get_heap()), m_ext_al(&xi.get_ext_heap()), m_data(0)
    {
        reset();
    }

    ScopedObjectCreator::ScopedObjectCreator(Type t, heap & al, ext_heap& eh)
        : m_type(t), m_al(&al), m_ext_al(&eh), m_data(0)
    {
        reset();
    }

    ScopedObjectCreator::~ScopedObjectCreator()
    {
        if (m_data)
        {
            m_type.methods().destruct(CommonObject(m_type, m_data));
            m_al->free(m_data, m_type.payload(), m_type.align());
            m_data = 0;
        }
    }


    CommonObject ScopedObjectCreator::release()
    {
        CommonObject ret = get();
        m_data = 0;
        return ret;
    }

    CommonObject ScopedObjectCreator::reset(){
        return reset(m_type);
    }

    CommonObject ScopedObjectCreator::reset(Type t){

        UninitObjectPtr ptr(t, *m_al);
        t.methods().construct(CommonObject(t, ptr.get()), *m_al, *m_ext_al);
        ptr.enableDtor();

        if (m_data)
        {
            t.methods().destruct(CommonObject(t, m_data));
            m_al->free(m_data, t.payload(), t.align());
        }
        m_data = ptr.release();
        m_type = t;
        return get();
    }

    CommonObject ScopedObjectCreator::adoptBy(Namespace ns, const file_path& name)
    {
        static const version_type latestVersion;
        return adoptBy(ns, name, latestVersion);
    }

    CommonObject ScopedObjectCreator::adoptBy(Namespace ns, const file_path& name, const version_type& /*version*/)
    {
        CommonObject current = get();
        XR_PRE_CONDITION(ImpAccessor<NamespaceImp>::getImp(ns)->objects.count(name) == 0);
        ImpAccessor<NamespaceImp>::getImp(ns)->objects[name] = CommonObject(m_type, m_data);
        m_data = 0;
        return current;
    }

    CommonObject ScopedObjectCreator::get() const
    {
        return CommonObject(m_type, m_data);
    }

    heap& ScopedObjectCreator::getHeap() const{
        return *m_al;
    }

    ext_heap& ScopedObjectCreator::getExtHeap() const{
        return *m_ext_al;
    }

    void ScopedObjectCreator::swap(ScopedObjectCreator& rhs)
    {
        using std::swap;
        swap(m_type, rhs.m_type);
        swap(m_al, rhs.m_al);
        swap(m_ext_al, rhs.m_ext_al);
        swap(m_data, rhs.m_data);
    }
	type::CommonObject cloneObject(type::ConstCommonObject from, heap & al, ext_heap& eh){
		if (!from.valid())
			return type::CommonObject();

		type::ObjectFactory factory(al, eh);
		return factory.clone(from);
	}
}}

