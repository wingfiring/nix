#include "typealiasimp.h"
#include <xirang2/type/typealias.h>
#include <xirang2/type/namespace.h>
#include "namespaceimp.h"

#include <memory>
#include "impaccessor.h"

namespace xirang2{ namespace type{
	TypeAlias::TypeAlias(TypeAliasImp* imp )
		: m_imp(imp)
	{}


	bool TypeAlias::valid() const
	{
		return m_imp != 0;
	}

	TypeAlias::operator bool() const
	{
		return m_imp != 0;
	}

	const file_path& TypeAlias::name() const
	{
		XR_PRE_CONDITION(valid());
		return m_imp->name;
	}

	const file_path& TypeAlias::typeName() const
	{
		XR_PRE_CONDITION(valid());
		return m_imp->typeName;
	}

	Type TypeAlias::type() const
	{
		XR_PRE_CONDITION(valid());
		return Type(m_imp->type);
	}

	int TypeAlias::compare (TypeAlias other) const
	{
        return comparePtr(m_imp, other.m_imp);
	}

	TypeAliasBuilder::TypeAliasBuilder()
		: m_imp(0)
	{
		renew();
	}

	TypeAliasBuilder::~TypeAliasBuilder()
	{
	}
	TypeAliasBuilder& TypeAliasBuilder::name(const file_path& alias)
	{
		m_imp->name = alias;
		return *this;
	}
	TypeAliasBuilder& TypeAliasBuilder::typeName(const file_path& typeName)
	{
		m_imp->typeName = typeName;
		return *this;
	}
	TypeAliasBuilder& TypeAliasBuilder::setType(Type t)
	{
        m_imp->type = ImpAccessor<TypeImp>::getImp(t);
		return *this;
	}

	TypeAliasBuilder& TypeAliasBuilder::renew()
	{
		unique_ptr<TypeAliasImp> tmp(new TypeAliasImp);
		tmp->parent = 0;
		tmp->type= 0;
		m_imp.swap(tmp);

		return *this;
	}
	TypeAlias TypeAliasBuilder::get() const
	{
		return TypeAlias(m_imp.get());
	}

    TypeAlias TypeAliasBuilder::adoptBy(Namespace ns)
    {
        XR_PRE_CONDITION(!m_imp->name.empty());
        XR_PRE_CONDITION(!ns.findAlias(m_imp->name).valid());

		unique_ptr<TypeAliasImp> tmp (new TypeAliasImp);
        TypeAlias current = get();

		auto origin = m_imp.get();
        ImpAccessor<NamespaceImp>::getImp(ns)->alias.insert(std::make_pair(m_imp->name, std::move(m_imp)));
        origin->parent = ImpAccessor<NamespaceImp>::getImp(ns);

        m_imp.swap(tmp);

        return current;
    }

    TypeAliasBuilder& TypeAliasBuilder::parent(Namespace ns)
    {
        XR_PRE_CONDITION(m_imp && !m_imp->parent && ns.valid());
        m_imp->parent = ImpAccessor<NamespaceImp>::getImp(ns);
        return *this;
    }

    TypeAlias TypeAliasBuilder::adoptBy()
    {
        XR_PRE_CONDITION(!m_imp->name.empty() && m_imp->parent);
        Namespace ns(m_imp->parent);

        XR_PRE_CONDITION(!ns.findAlias(m_imp->name).valid());

        unique_ptr<TypeAliasImp> tmp (new TypeAliasImp);
        TypeAlias current = get();

        ImpAccessor<NamespaceImp>::getImp(ns)->alias.insert(std::make_pair(m_imp->name, std::move(m_imp)));
        m_imp.swap(tmp);

        return current;
    }


    Namespace TypeAliasBuilder::getParent() const
    {
        return m_imp->parent;
    }

    const file_path& TypeAliasBuilder::getName() const
    {
        return m_imp->name;
    }

    const file_path& TypeAliasBuilder::getTypeName() const
    {
        return m_imp->typeName;
    }
}}

