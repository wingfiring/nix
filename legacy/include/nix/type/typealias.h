#ifndef XIRANG2_TYPE_ALIAS_H__
#define XIRANG2_TYPE_ALIAS_H__

#include <xirang2/type/xrfwd.h>
#include <xirang2/type/type.h>

namespace xirang2 { namespace type{
	class TypeAliasImp;

    /// type alias
	class XR_API TypeAlias
	{
		public:
			TypeAlias(TypeAliasImp* imp = 0);

			bool valid() const;

			operator bool () const;

			const file_path& name() const;
			const file_path& typeName() const;
			Type type() const;
			int compare (TypeAlias other) const;
		private:
			TypeAliasImp* m_imp;
	};
	DEFINE_COMPARE (TypeAlias);

	class XR_API TypeAliasBuilder
	{
	public:
		TypeAliasBuilder();
		~TypeAliasBuilder();
		TypeAliasBuilder& name(const file_path& alias);
		TypeAliasBuilder& typeName(const file_path& typeName);
		TypeAliasBuilder& setType(Type t);
		TypeAliasBuilder& renew();
        TypeAliasBuilder& parent(Namespace ns);
        TypeAlias adoptBy();
        TypeAlias adoptBy(Namespace ns);

        TypeAlias get() const;
        Namespace getParent() const;
        const file_path& getName() const;
        const file_path& getTypeName() const;

	private:
		unique_ptr<TypeAliasImp> m_imp;

		friend class NamespaceBuilder;
	};
}}
#endif //end XIRANG2_TYPE_ALIAS_H__
