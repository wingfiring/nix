#include <xirang2/type/type.h>
#include <xirang2/type/typebinder.h>
#include <xirang2/type/namespace.h>
#include <xirang2/type/object.h>

#include "./typeimp.h"
#include "./namespaceimp.h"
#include "./impaccessor.h"

namespace xirang2{ namespace type{

	TypeItem::TypeItem (TypeItemImp * p):m_imp (p) { }
    TypeItem::TypeItem (TypeItemImp & p):m_imp (&p) { }

	TypeItem::operator bool () const
	{
		return m_imp != 0;
	}

	bool TypeItem::valid () const
	{
		return m_imp != 0;
	}

	Type TypeItem::type () const
	{
		XR_PRE_CONDITION (valid ());
		return Type (m_imp->type);
	}

	const string & TypeItem::name () const
	{
		XR_PRE_CONDITION (valid ());
		return m_imp->name;
	}

	const file_path & TypeItem::typeName () const
	{
		XR_PRE_CONDITION (valid ());
		return m_imp->typeName;
	}

    size_t TypeItem::index() const
    {
        XR_PRE_CONDITION (valid ());
        return m_imp->index;
    }

	std::size_t TypeItem::offset () const
	{
		XR_PRE_CONDITION (valid () );
		return m_imp->offset;
	}

	bool TypeItem::isResolved () const
	{
		XR_PRE_CONDITION (valid ());
		return m_imp->type != 0 && m_imp->type->isMemberResolved();
	}

	int TypeItem::compare (const TypeItem& rhs) const
	{
        return comparePtr(m_imp, rhs.m_imp);
	}


	//
	// TypeArg
	//
	TypeArg::TypeArg (TypeArgImp * imp ) : m_imp(imp){}
	TypeArg::TypeArg (TypeArgImp & imp ) : m_imp(&imp){}

	bool TypeArg::valid () const
	{
		return m_imp != 0;
	}

	Type TypeArg::type () const
	{
		XR_PRE_CONDITION(valid());
		return Type(m_imp->type);
	}

	const string & TypeArg::name () const
	{
		XR_PRE_CONDITION(valid());
		return m_imp->name;
	}

	const file_path & TypeArg::typeName () const
	{
		XR_PRE_CONDITION(valid());
		return m_imp->typeName;
	}

	bool TypeArg::isBound () const
	{
		XR_PRE_CONDITION(valid());
		return m_imp->type != 0;
	}

	int TypeArg::compare (const TypeArg & rhs) const
	{
        return comparePtr(m_imp, rhs.m_imp);
	}

	//
	// Type
	//
	void TypeImp::releaseData(TypeImp* t, void* data){
		if (!data)	return;
		auto& hp = xirang2::memory::get_global_heap();
		t->methods->destruct(CommonObject(Type(t), data));
		hp.free(data, t->payload, t->alignment);
	}
	CommonObject TypeImp::createIntenalObject(TypeImp* t, void* & dataPtr){
		XR_PRE_CONDITION(!dataPtr && t);
		ScopedObjectCreator obj(Type(t), memory::get_global_heap(), memory::get_global_ext_heap());
		dataPtr = obj.release().data();

		return CommonObject(t, dataPtr);
	}

	Type::Type (TypeImp * imp):m_imp (imp) { }
	Type::Type (TypeImp & imp):m_imp (&imp) { }

	bool Type::valid () const
	{
		return m_imp != 0;
	}

    Type::operator bool() const
	{
		return m_imp != 0;
	}

	//type name without namespace name
	const file_path & Type::name () const
	{
		XR_PRE_CONDITION (valid () );
		return m_imp->name;
	}

	//identifier name
	file_path Type::fullName () const
	{
		XR_PRE_CONDITION (valid ());

		std::vector<NamespaceImp*> parents;

		for (NamespaceImp * ns = m_imp->parent;
				ns != 0; ns = ns->parent)
		{
			parents.push_back(ns);
		}

		file_path ret("/", pp_none);
		for (auto itr = parents.rbegin(); itr != parents.rend(); ++itr){
			ret /= (*itr)->name;
		}
		ret /= name();
		return ret;
	}

    file_path Type::versionedFullName() const {
        return file_path(string(fullName().str() << literal("#") << sha1_to_string(version().id)), pp_none);
    }

    Namespace Type::parent() const
    {
        XR_PRE_CONDITION(valid());
        return Namespace(m_imp->parent);
    }

	//if user didn't name a type, it has an internal name.
	bool Type::isInterim () const
	{
		XR_PRE_CONDITION (valid () );
		return m_imp->name.empty () || m_imp->name.str()[0] == '~';
	}

	bool Type::isMemberResolved() const
	{
		XR_PRE_CONDITION (valid ());
		return m_imp->isMemberResolved();
	}

	//true if no sub-item is unresolved.
	size_t Type::unresolvedArgs () const
	{
		XR_PRE_CONDITION (valid ());
		return m_imp->unresolvedArgs;
	}

	bool Type::isComplete() const
	{
		XR_PRE_CONDITION (valid ());
		return m_imp->isMemberResolved()
			&& m_imp->unresolvedArgs == 0;

	}

	bool Type::hasModel () const
	{
		XR_PRE_CONDITION(valid());
		return !m_imp->modelName.empty();
	}

	Type Type::model () const
	{
		XR_PRE_CONDITION(valid());
		return Type(m_imp->modelType);
	}

	const file_path& Type::modelName () const
	{
		XR_PRE_CONDITION(valid());
		return m_imp->modelName;
	}

	//alignment
	std::size_t Type::align () const
	{
		XR_PRE_CONDITION (valid ());
		return m_imp->alignment;
	}
	//object payload.
	std::size_t Type::payload () const
	{
		XR_PRE_CONDITION (valid ());
		return m_imp->payload;
	}

	bool Type::isPod () const
	{
		XR_PRE_CONDITION (valid ());
		return isMemberResolved() && m_imp->isPod;
	}

	TypeMethods & Type::methods () const
	{
		XR_PRE_CONDITION (valid ());
		XR_PRE_CONDITION (m_imp->methods != 0);
		return *m_imp->methods;
	}

	//first all sub-item. base + memebers.
	std::size_t Type::memberCount () const
	{
		XR_PRE_CONDITION (valid ());
		return m_imp->members();
	}

	TypeItemRange Type::members () const
	{
		XR_PRE_CONDITION (valid ());

		return TypeItemRange(
            TypeItemRange::iterator (TypeItemIteratorImp(m_imp->items.begin())),
			TypeItemRange::iterator (TypeItemIteratorImp(m_imp->items.end()))
				);
	}

	TypeItem Type::member (std::size_t idx) const
	{
		XR_PRE_CONDITION (valid () && idx < m_imp->members() );
		return TypeItem(&m_imp->items[idx]);
	}

	TypeItem Type::member (const string& name) const
	{
		XR_PRE_CONDITION (valid ());
		for (std::vector<TypeItemImp>::iterator itr(m_imp->items.begin());
				itr != m_imp->items.end(); ++itr)
		{
			if (name == itr->name)
				return TypeItem(&*itr);
		}
		return TypeItem();
	}

	std::size_t Type::argCount () const
	{
		XR_PRE_CONDITION (valid ());
		return m_imp->typeArgs.size();
	}

	TypeArgRange Type::args () const
	{
		XR_PRE_CONDITION (valid ());
		return TypeArgRange(
            TypeArgRange::iterator (TypeArgIteratorImp(m_imp->typeArgs.begin())),
			TypeArgRange::iterator (TypeArgIteratorImp(m_imp->typeArgs.end()))
				);
	}
	TypeArg Type::arg (std::size_t idx) const
	{
		XR_PRE_CONDITION (valid ());
		XR_PRE_CONDITION (idx < argCount());
		return TypeArg(&m_imp->typeArgs[idx]);
	}

	TypeArg Type::arg(const string& name) const
	{
		XR_PRE_CONDITION (valid ());

		for (std::vector<TypeArgImp>::iterator itr = m_imp->typeArgs.begin();
				itr != m_imp->typeArgs.end(); ++itr)
		{
			if (name == itr->name)
				return TypeArg(&*itr);
		}
		return TypeArg();
	}

    Type Type::locateType(const file_path& n) const
	{
		XR_PRE_CONDITION (valid ());

		if (n.is_absolute())
            return parent().valid() ? parent().locateType(n) : Type();

        auto itr = n.begin();
		auto end = n.end();

        Type res;

        if (itr != end)
        {
            TypeArg theArg = arg(itr->str());
            if (!theArg.valid())
                return parent().valid() ? parent().locateType(n) : Type();
            else
                res = theArg.type();
            ++itr;
        }

		for (; res.valid() && itr != end; ++itr)
		{
			TypeArg arg = res.arg(itr->str());
            res = arg.valid() ? arg.type() : Type();
		}

		return res;
	}

	Type Type::staticType() const{
		XR_PRE_CONDITION(valid());
		return Type(m_imp->staticType);
	}

	ConstCommonObject Type::staticData() const{
		XR_PRE_CONDITION(valid());
		return ConstCommonObject(m_imp->staticData ? m_imp->staticType : Type(), m_imp->staticData);
	}

	ConstCommonObject Type::prototype() const{
		XR_PRE_CONDITION(valid());
		return ConstCommonObject(m_imp->prototype? *this : Type(), m_imp->prototype);
	}

	CommonObject Type::userData(TypeMetaAccessType acc /* = tma_create_if_not_exist */) const{
		XR_PRE_CONDITION(valid());
		bool dataExist = m_imp->userData != 0;
		switch (acc){
			case tma_query:
				break;
			case tma_force_create:
				XR_PRE_CONDITION(!m_imp->userData && m_imp->userType);
				// fall through
			case tma_create_if_not_exist:
				if (m_imp->userType && !m_imp->userData){
					ScopedObjectCreator obj(Type(m_imp->userType), memory::get_global_heap(), memory::get_global_ext_heap());
					m_imp->userData = obj.release().data();
				}
				break;
			default:
				XR_PRE_CONDITION(false && "invalid acc");
		}

		m_imp->userDataIsOwned = m_imp->userDataIsOwned || (!dataExist && m_imp->userData);
		return CommonObject(m_imp->userData? m_imp->userType : 0, m_imp->userData);
	}

	void Type::setUserData(CommonObject newData) const{
		XR_PRE_CONDITION(valid());
		m_imp->releaseData(m_imp->userType, m_imp->userData);
		m_imp->userType = newData.type().m_imp;
		m_imp->userData = newData.data();
		m_imp->userDataIsOwned = false;
	}

	bool Type::isOwnedUserData() const{
		XR_PRE_CONDITION(valid());
		return m_imp->userDataIsOwned;
	}

	int Type::compare (const Type& rhs) const
	{
        return comparePtr(m_imp, rhs.m_imp);
	}
	const version_type& Type::version() const{
		XR_PRE_CONDITION(valid());
		return m_imp->version;
	}

	void Type::releaseAllData(){
		XR_PRE_CONDITION(valid());
		return m_imp->releaseAllData();
	}
	TypeSynonym::TypeSynonym(TypeSynonymImp* imp) : m_imp(imp){}
	Type TypeSynonym::current() const{
		XR_PRE_CONDITION(valid());
		return Type(m_imp->current);
	}
	Type TypeSynonym::setCurrent(Type t) const{
		XR_PRE_CONDITION(valid());
		XR_PRE_CONDITION(m_imp->items.count(t.version()) !=0);
		Type old(m_imp->current);
		m_imp->current = ImpAccessor<TypeImp>::getImp(t);
		return old;
	}
	Type TypeSynonym::setCurrent(const version_type& v) const{
		XR_PRE_CONDITION(valid());
		XR_PRE_CONDITION(m_imp->items.count(v) !=0);
		auto pos = m_imp->items.find(v);
		m_imp->current = Type(pos->second.get());
		return Type(m_imp->current);
	}
	TypeRange TypeSynonym::items() const{
		XR_PRE_CONDITION(valid());
        return TypeRange (
            TypeRange::iterator(TypeIterator(m_imp->items.begin())),
            TypeRange::iterator(TypeIterator(m_imp->items.end()))
            );
	}
	bool TypeSynonym::valid() const{
		return m_imp != 0;
	}

	TypeBuilder::TypeBuilder(TypeMethods* methods )
		: m_imp(0)
	{
		renew(methods);
	}
	TypeBuilder::~TypeBuilder()
	{
	}

	TypeBuilder& TypeBuilder::name(const file_path& name)
	{
		XR_PRE_CONDITION(m_imp && !name.empty());
		m_imp->name = name;
		return *this;
	}

	TypeBuilder& TypeBuilder::modelFrom(Type from)
	{
		XR_PRE_CONDITION(from.valid() );
		XR_PRE_CONDITION(m_imp);
        XR_PRE_CONDITION(m_stage < st_model);

        ImpAccessor<TypeImp>::getImp(from)->modelTo(*m_imp);
        m_stage = st_model;

		return *this;
	}

	TypeBuilder& TypeBuilder::setArg(const string& arg, const file_path& typeName, Type t)
	{
        XR_PRE_CONDITION(!arg.empty());
        XR_PRE_CONDITION(m_stage <= st_arg);

        TypeArgImp* target = 0;

        for (std::vector<TypeArgImp>::iterator itr = m_imp->typeArgs.begin();
				itr != m_imp->typeArgs.end(); ++itr)
		{
			if (arg == itr->name)
            {
                target = &*itr;
            }
		}

        if (target) // replace
        {
            if (!target->type)
                --m_imp->unresolvedArgs;
            target->typeName = typeName;
            target->type = ImpAccessor<TypeImp>::getImp(t);
            if (!target->type)
                ++m_imp->unresolvedArgs;
        }
        else
        {
            m_imp->typeArgs.resize(m_imp->typeArgs.size() + 1);
            m_imp->typeArgs.back().name = arg;
            m_imp->typeArgs.back().typeName = typeName;

            m_imp->typeArgs.back().type = ImpAccessor<TypeImp>::getImp(t);
            if (m_imp->typeArgs.back().type == 0) m_imp->unresolvedArgs++;
        }
        m_stage = st_arg;
		return *this;
	}

	TypeBuilder& TypeBuilder::addMember(const string& name, const file_path& typeName, Type t)
	{
		XR_PRE_CONDITION(m_imp);
        XR_PRE_CONDITION(!name.empty() && !Type(*m_imp).member(name).valid());
        XR_PRE_CONDITION(m_stage <= st_member);

		m_imp->items.resize(m_imp->items.size() + 1);
		TypeItemImp& m = m_imp->items.back();
		m.name = name;
		m.typeName = typeName;
		m.type = ImpAccessor<TypeImp>::getImp(t);
        m.index = m_imp->items.size() - 1;

		TypeItem tim(&m);
		m_imp->methods->nextLayout(tim, m_imp->payload, m_offset, m_imp->alignment, m_imp->isPod);

		if (t.valid())
		{
			m.offset = m_imp->payload - t.payload();
		}
		else
			m.offset = Type::no_size;


        m_stage = st_member;
		return *this;
	}
	TypeBuilder& TypeBuilder::setStaticType(Type t){
        XR_PRE_CONDITION(!m_imp->staticData && (!t.valid() || t.isMemberResolved()));
		m_imp->staticType = ImpAccessor<TypeImp>::getImp(t);
		return *this;
	}

	TypeBuilder& TypeBuilder::endBuild(bool autoResolve)
	{
        XR_PRE_CONDITION(m_stage < st_end);

        if (autoResolve && !get().isMemberResolved()){

            m_imp->methods->beginLayout(m_imp->payload, m_offset, m_imp->alignment, m_imp->isPod);
            for (std::vector < TypeItemImp >::iterator itr = m_imp->items.begin(); itr != m_imp->items.end(); ++itr)
            {
                if (!itr->type)
                {
                    Type t = get().locateType(itr->typeName);
                    itr->type = ImpAccessor<TypeImp>::getImp(t);
                }

                if (itr->type && itr->type->isMemberResolved())
                {
                    TypeItem tim(&*itr);
                    itr->offset = m_offset;
                    m_imp->methods->nextLayout(tim, m_imp->payload, m_offset, m_imp->alignment, m_imp->isPod);
                }
                else
                {
                    m_imp->payload = Type::no_size;
                    break;
                }
            }
        }
        auto origin = m_imp.get();
        origin->version = calculateTypeVersion(get());
        m_stage = st_end;

        return *this;
	}

	Type TypeBuilder::get() const
	{
		return Type(m_imp.get());
	}

	TypeBuilder& TypeBuilder::renew(TypeMethods* methods )
	{
		unique_ptr<TypeImp> tmp (new TypeImp);
        tmp->methods = methods == 0? &DefaultMethods() : methods;
		m_imp.swap(tmp);
        m_imp->methods->beginLayout(m_imp->payload, m_offset, m_imp->alignment, m_imp->isPod);

        m_stage = st_renew;
		return *this;
	}

    Type TypeBuilder::adoptBy(Namespace ns)
    {
        XR_PRE_CONDITION(!m_imp->name.empty());
        XR_PRE_CONDITION(m_stage == st_end);

        unique_ptr<TypeImp> tmp (new TypeImp);
        Type current = get();

		auto origin = m_imp.get();

		auto nsimp = ImpAccessor<NamespaceImp>::getImp(ns);
		auto& s = nsimp->types[m_imp->name];
		if (s.items.count(origin->version) == 0){
			s.items.emplace(origin->version, std::move(m_imp));
			s.current = current;
		}
        origin->parent = nsimp;

        m_imp.swap(tmp);
        m_stage = st_renew;

        return current;
    }

    TypeBuilder& TypeBuilder::parent(Namespace ns)
    {
        m_imp->parent = ImpAccessor<NamespaceImp>::getImp(ns);
        return *this;
    }

    Type TypeBuilder::adoptBy()
    {
        XR_PRE_CONDITION(!m_imp->name.empty());
        Namespace ns(m_imp->parent);
		return adoptBy(ns);
    }

    const file_path& TypeBuilder::getName() const
    {
        return m_imp->name;
    }
    Namespace TypeBuilder::getParent() const
    {
        return m_imp->parent;
    }

    TypeBuilder::stage TypeBuilder::getStage() const
    {
        return this->m_stage;
    }
	CommonObject TypeBuilder::createPrototype(){
        XR_PRE_CONDITION(m_stage < st_prototype);
        XR_PRE_CONDITION(get().isMemberResolved());
        XR_PRE_CONDITION(!m_imp->prototype);

        m_stage = st_prototype;
		return m_imp->createIntenalObject(m_imp.get(), m_imp->prototype);
	}
	CommonObject TypeBuilder::createStaticData(){
        XR_PRE_CONDITION(get().staticType().valid());
        XR_PRE_CONDITION(get().staticType().isMemberResolved());
		XR_PRE_CONDITION(!m_imp->staticData);
		return m_imp->createIntenalObject(m_imp->staticType, m_imp->staticData);
	}

}}


