#include "namespaceimp.h"

#include <xirang2/type/namespace.h>
#include <xirang2/type/type.h>
#include <xirang2/type/typealias.h>
#include <xirang2/type/object.h>
#include "impaccessor.h"

namespace xirang2{ namespace type{

	// ************
	//  Namespace
	// ************
	Namespace::Namespace (NamespaceImp * imp):m_imp (imp)
	{
	}

	const file_path & Namespace::name () const
	{
		XR_PRE_CONDITION (valid ());
		return m_imp->name;
	}

    file_path Namespace::fullName() const
    {
		file_path ret("/", pp_none);

        std::vector <Namespace> parents;
		for(auto ns = *this; ns.valid(); ns = ns.parent())
			parents.push_back(ns);

        for (auto itr(parents.rbegin()),
            end (parents.rend()); itr != end; ++itr)
        {
			ret /= itr->name();
        }
		return ret;
    }
	Type Namespace::locateRealType(const file_path& n, const version_type& ver) const{
		auto ns = locateNamespace(n.parent());
		return ns.valid() ? ns.findRealType(n.filename(), ver) : Type();
	}
	Type Namespace::locateType(const file_path& n) const
	{
		XR_PRE_CONDITION (valid ());

		if (n.empty())	return Type();

		Namespace cur = *this;
		auto itr = n.begin();
		auto end = n.end();

        if (n.is_absolute()) {
            cur = root();
			++itr;
        }
        else //find first section
        {
            cur = *this;
            for (file_path id = *itr; cur.valid() ; cur = cur.parent())
            {
                if (cur.findNamespace(id).valid()
                    || cur.findType(id).valid()
                    || cur.findAlias(id).valid()
                    )
                    break;
            }
        }

		Type res;
		for (; itr != end; ++itr)
		{
			file_path id = *itr;

			Namespace tns = cur.findNamespace(id);
			if (tns.valid())
			{
				cur = tns;
				continue;
			}
			res = cur.findType(id);
			if (res.valid())
			{
				++itr;
				break;
			}
			TypeAlias tas = cur.findAlias(id);
			if (tas.valid())
			{
				res = tas.type();
				++itr;
				break;
			}
			return Type();
		}
		for (; res.valid() && itr != end; ++itr)
		{
			TypeArg arg = res.arg(itr->str());
			res = arg.valid() ? arg.type() : Type();
		}

		return res;
	}

	Namespace Namespace::locateNamespace(const file_path& n) const
	{
		XR_PRE_CONDITION (valid ());
		if (n.is_root())
			return this->root();
		if (n.empty())
			return *this;

		auto itr = n.begin();
		auto end = n.end();

		bool isAbsolute = n.is_absolute();
		Namespace cur = isAbsolute? root() : *this;
		if (isAbsolute)
			++itr;
		else
			for (file_path id = *itr; cur.valid() ; cur = cur.parent())
				if (cur.findNamespace(id).valid()) {
					break;
				}

		for (; cur.valid() && itr != end; ++itr)
			cur = cur.findNamespace(*itr);

		return cur;
	}

	Type Namespace::findType (const file_path & t) const
	{
		XR_PRE_CONDITION (valid ());
		Type result = findRealType(t);
		if(result.valid())
		   return result;
		TypeAlias alias = findAlias(t);
		return alias.valid() ? alias.type() : Type();
	}

	Type Namespace::findRealType (const file_path & t) const
	{
		XR_PRE_CONDITION (valid ());
		auto pos = m_imp->types.find (t);

		return pos == m_imp->types.end() ? Type() : pos->second.current;
	}
	Type Namespace::findRealType (const file_path & t, const version_type& ver) const{
		XR_PRE_CONDITION (valid ());
		auto pos = m_imp->types.find (t);

		if(pos == m_imp->types.end())
			return Type();

		auto p = pos->second.items.find(ver);

		if (p == pos->second.items.end())
			return Type();

		return Type(p->second.get());

	}

	Namespace Namespace::findNamespace (const file_path & ns) const
	{
		XR_PRE_CONDITION (valid ());
		auto pos =
			m_imp->children.find (ns);
		return pos == m_imp->children.end ()
			? Namespace () : Namespace (pos->second.get());
	}

	TypeAlias Namespace::findAlias (const file_path & t) const
	{
		XR_PRE_CONDITION (valid ());
		auto pos = m_imp->alias.find (t);

		return pos == m_imp->alias.end() ? TypeAlias() : TypeAlias(pos->second.get());
	}

	TypeSynonymRange Namespace::types () const
	{
		XR_PRE_CONDITION (valid ());
        return TypeSynonymRange (
            TypeSynonymRange::iterator(TypeSynonymIterator(m_imp->types.begin())),
            TypeSynonymRange::iterator(TypeSynonymIterator(m_imp->types.end()))
            );
	}
	TypeRange Namespace::types (const file_path& n) const{
		XR_PRE_CONDITION (valid ());
		auto pos = m_imp->types.find(n);
		if (pos == m_imp->types.end())
			return TypeRange();

        return TypeRange (
            TypeRange::iterator(TypeIterator(pos->second.items.begin())),
            TypeRange::iterator(TypeIterator(pos->second.items.end()))
            );
	}

	TypeSynonym Namespace::getTypeSynonym(const file_path& n) const{
		XR_PRE_CONDITION (valid ());
		auto pos = m_imp->types.find(n);
		if (pos == m_imp->types.end())
			return TypeSynonym();

		return TypeSynonym(&pos->second);
	}

	NamespaceRange Namespace::namespaces () const
	{
		XR_PRE_CONDITION (valid ());
        return NamespaceRange (
            NamespaceRange::iterator(NamespaceIteratorImp(m_imp->children.begin())),
            NamespaceRange::iterator(NamespaceIteratorImp(m_imp->children.end()))
            );
	}

	TypeAliasRange Namespace::alias () const
	{
		XR_PRE_CONDITION (valid ());
		return TypeAliasRange (
            TypeAliasRange::iterator(TypeAliasIteratorImp(m_imp->alias.begin())),
            TypeAliasRange::iterator(TypeAliasIteratorImp(m_imp->alias.end()))
            );
	}

	ObjectRange Namespace::objects () const
	{
		XR_PRE_CONDITION (valid ());
		return ObjectRange (
            ObjectRange::iterator(ObjIteratorImp(m_imp->objects.begin())),
            ObjectRange::iterator(ObjIteratorImp(m_imp->objects.end()))
            );
	}

    NameValuePair Namespace::findObject(const file_path& name) const
    {
        const static version_type last_version;
        return findObject(name, last_version);
    }

    NameValuePair Namespace::findObject(const file_path& name, const version_type& /*version*/ ) const
    {
		auto pos = m_imp->objects.find(name);
        NameValuePair ret = {0,CommonObject()};
        if (pos != m_imp->objects.end())
        {
            ret.name = &pos->first;
            ret.value = pos->second;
        }
        return ret;
    }

	bool Namespace::empty () const
	{
		XR_PRE_CONDITION (valid ());
		return m_imp->types.empty ()
			&& m_imp->children.empty () && m_imp->objects.empty ()
			&& m_imp->alias.empty();
	}

	Namespace Namespace::parent () const
	{
		XR_PRE_CONDITION (valid ());
		return Namespace (m_imp->parent);
	}

	Namespace Namespace::root() const
	{
		XR_PRE_CONDITION (valid ());

		NamespaceImp* rt = m_imp;
		for (; rt->parent != 0; rt = rt->parent)
			;
		return Namespace(rt);
	}

	int Namespace::compare (Namespace other) const
	{
        return comparePtr(m_imp, other.m_imp);
	}

	bool Namespace::valid() const
	{
		return m_imp != 0;
	}

	Namespace::operator bool() const
	{
		return m_imp != 0;
	}

    FindNamespaceResult findNamespace(Namespace from, sub_file_path path){
        XR_PRE_CONDITION (from.valid ());
        
        auto itr = path.begin();
        auto end = path.end();
        if (path.is_absolute()) ++itr;
        
        Namespace cur = from;
        
        for (; itr != end; ++itr)
        {
            auto result = cur.findNamespace(*itr);
            if (!result.valid())
                break;
            cur = result;
        }
        xirang2::sub_file_path rest (itr == end ? path.str().end() : itr->str().begin(), path.str().end());
        FindNamespaceResult result = {cur, rest};
        return result;
    }
    Namespace createNamespaceRecursively(Namespace from, sub_file_path path){
        XR_PRE_CONDITION(!path.empty());
        
        auto res = findNamespace(from, path);
        XR_PRE_CONDITION(!res.rest.empty());
        
        auto pos = res.rest.begin();
        NamespaceBuilder ns_builder;
        ns_builder.name(*pos);
        if (++pos != res.rest.end()){
            sub_file_path left(pos->str().begin(), res.rest.str().end());
            ns_builder.createChild(left);
        }
        ns_builder.adoptBy(res.match);
        auto leaf = findNamespace(from, res.rest);
        XR_PRE_CONDITION(leaf.rest.empty() && leaf.match.valid());
        return leaf.match;
    }
    Namespace createOrFindNamespaceRecursively(Namespace from, sub_file_path path){
        auto res = findNamespace(from, path);
        
        if (res.rest.empty())
            return res.match;
        return createNamespaceRecursively(res.match, res.rest);
    }
    NamespaceBuilder& NamespaceBuilder::createChild(const file_path& path)
    {
        XR_PRE_CONDITION(!path.is_absolute());
        auto itr = path.begin();
		auto end = path.end();

        for (Namespace current(get()); itr != end; ++itr)
        {
            auto name = *itr;
            if (name.empty())
                XR_THROW(invalid_namespace_path)("path contains empty name.")(path.str());
            Namespace next = current.findNamespace(name);
            if (!next.valid())
                next = NamespaceBuilder().name(name).adoptBy(current);
            current = next;
        }
        return *this;
    }

	NamespaceBuilder& NamespaceBuilder::adopt(TypeBuilder& tb)
	{
		XR_PRE_CONDITION(m_imp && tb.get().valid());

		tb.adoptBy(get());

		return *this;
	}

	NamespaceBuilder& NamespaceBuilder::adopt(NamespaceBuilder& tb)
	{
		XR_PRE_CONDITION(m_imp && tb.get().valid());

        tb.adoptBy(get());

		return *this;
	}

	NamespaceBuilder& NamespaceBuilder::adopt(TypeAliasBuilder& tb)
	{
		XR_PRE_CONDITION(m_imp && tb.get().valid());

        tb.adoptBy(get());

        return *this;
	}

	Namespace NamespaceBuilder::adoptBy(Namespace ns)
	{
		XR_PRE_CONDITION(m_imp && ns.valid());
        XR_PRE_CONDITION(!m_imp->parent || m_imp->parent == ImpAccessor<NamespaceImp>::getImp(ns));

        unique_ptr<NamespaceImp> pnew (new NamespaceImp);

		//TODO: merge namespace content
		auto name = get().name();
		XR_PRE_CONDITION(!name.empty() && !ns.findNamespace(name).valid());

        Namespace current = get();
		auto origin = m_imp.get();
        ImpAccessor<NamespaceImp>::getImp(ns)->children.insert(std::make_pair(name, std::move(m_imp)));
		origin->parent = ImpAccessor<NamespaceImp>::getImp(ns);
		m_imp.swap(pnew);

        return current;
	}

	template<typename Cont>
	void merge_cont_(Cont& from, Cont& to, NamespaceImp* parent){
		for (auto& i : from){
			i.second->parent = parent;
			to.emplace(i.first, std::move(i.second));
		}
		from.clear();
	}
	template<typename Cont>
	void merge_cont_(Cont& from, Cont& to){
		for (auto& i : from)
			to.emplace(i.first, std::move(i.second));
		from.clear();
	}

	template<typename Cont>
	void merge_types_(Cont& from, Cont& to, NamespaceImp* parent){
		for (auto& i : from){
			auto& d = to[i.first];
			for (auto& t : i.second.items){
				t.second->parent = parent;
				XR_PRE_CONDITION(d.items.count(t.first) == 0);
				d.items.emplace(t.first, std::move(t.second));
			}
			d.current = i.second.current;
		}
		from.clear();
	}

	void merge_namespace_children_(NamespaceImp& from, NamespaceImp& to){
		for (auto& ts: from.types){
			auto& tp_synonym = to.types[ts.first];
			for (auto& t: ts.second.items){
				if (tp_synonym.items.count(t.second->version) == 0){
					t.second->parent = &to;
					tp_synonym.items.emplace(t.second->version, std::move(t.second));
				}
			}
			tp_synonym.current = ts.second.current;
		}
		from.types.clear();

		for (auto& ta : from.alias){
			ta.second->parent = &to;
			to.alias.emplace(ta.first, std::move(ta.second));
		}
		from.alias.clear();

		for (auto& i : from.objects){
			to.objects.emplace(i.first, i.second);
		}
		from.objects.clear();

		for (auto& i : from.children){
			i.second->parent = &to;
			auto pos = to.children.find(i.first);
			if (pos == to.children.end()){
				to.children.emplace(i.first, std::move(i.second));
			}
			else {
				merge_namespace_children_(*i.second, *pos->second);
			}
		}
		from.children.clear();
	}

	template<typename Cont>
		bool has_intersect(const Cont& lhs, const Cont& rhs){
			auto il = lhs.begin();
			auto el = lhs.end();
			auto ir = rhs.begin();
			auto er = rhs.end();

			for (; il != el && ir != er; ){
				if (il->first < ir->first)
					++il;
				else if(ir->first < il->first)
					++ir;
				else
					return true;
			}
			return false;
		}
	bool check_name_conflict_(const NamespaceImp& left, const NamespaceImp& right){
		if (//has_intersect(left.types, right.types) ||
				 has_intersect(left.alias, right.alias) ||
				 has_intersect(left.objects, right.objects)
		   )
			return false;

		auto il = left.children.begin();
		auto el = left.children.end();
		auto ir = right.children.begin();
		auto er = right.children.end();

		// same namespace name is ok
		for (; il != el && ir != er; ){
			if (il->first < ir->first)
				++il;
			else if(ir->first < il->first)
				++ir;
			else if (!check_name_conflict_(*il->second, *ir->second))
				return false;
			else
			{ ++il; ++ir;}
		}
		return true;
	}

	NamespaceBuilder& NamespaceBuilder::adoptChildrenBy(Namespace ns)
	{
		XR_PRE_CONDITION(m_imp && ns.valid());
		XR_PRE_CONDITION(!m_imp->parent);

		NamespaceImp* target = ImpAccessor<NamespaceImp>::getImp(ns);

		XR_PRE_CONDITION(check_name_conflict_(*m_imp, *target));

		unique_ptr<NamespaceImp> pnew (new NamespaceImp);
		merge_namespace_children_(*m_imp, *target);
		m_imp.swap(pnew);

		return *this;
	}
	NamespaceBuilder& NamespaceBuilder::parent(Namespace ns)
	{
		XR_PRE_CONDITION(m_imp && !m_imp->parent && ns.valid());
		m_imp->parent = ImpAccessor<NamespaceImp>::getImp(ns);
		return *this;
	}

	Namespace NamespaceBuilder::adoptBy()
	{
		XR_PRE_CONDITION(m_imp && m_imp->parent);
		return adoptBy(Namespace(m_imp->parent));
	}

	NamespaceBuilder::NamespaceBuilder()
		: m_imp(new NamespaceImp)
	{	}

	NamespaceBuilder::~NamespaceBuilder()
	{
	}

	NamespaceBuilder& NamespaceBuilder::name(const file_path& n)
	{
		XR_PRE_CONDITION(m_imp);
		m_imp->name = n;
		return *this;
	}
	Namespace NamespaceBuilder::get() const
	{
		return Namespace(m_imp.get());
	}

	const file_path& NamespaceBuilder::getName() const
	{
		return m_imp->name;
	}
	Namespace NamespaceBuilder::getParent() const
	{
		return Namespace (m_imp->parent);
	}

	void NamespaceBuilder::swap(NamespaceBuilder& other)
	{
		std::swap(m_imp, other.m_imp);
	}
}}
