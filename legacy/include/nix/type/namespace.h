#ifndef XIRANG2_NAMESPACE_H__
#define XIRANG2_NAMESPACE_H__

#include <xirang2/type/object.h>

namespace xirang2{ namespace type{
	//namespace
	class NamespaceImp;

	//TODO: move most of member function as non-member functions, such as locateType. we should define a minimal core function set.
	// Namespace
	class XR_API Namespace
	{
		public:
            /// ctor
            /// if imp == 0, then valid() is false
			Namespace (NamespaceImp * imp = 0);

            /// get namespace name, can be empty string
            /// \pre valid()
			const file_path & name () const;

            /// get namespace name path. return '.' for root if dim is '.'
            /// \pre valid()
            /// \post return value start with dim
			/// \note with version section, <path>/#<version>
			file_path  fullName () const;

            /// locate the type with given name. the name can be a name path
            /// \param n name path. assume dim is '.', then
            ///  .xxx.yyy : search namespace xxx in root namespace, if found, search type or type alias yyy in namespace xxx, if found return the result.
            ///             otherwise returns a !valid() type. the starting dim means absolute name and search from root.
            ///  yyy      : search type or type alias yyy in current namespace, if found, return. if not found, search in the parent namespace, parent of parent namespace until root,
            ///             if found return immediately. if not found, return a !valid() type.
            ///  xxx.yyy  : search namespace xxx in the same way as above yyy. then search type yyy in the namespace xxx.
            /// \param separator of name path, suggestion is '.' or '/'
            /// \pre valid()
            /// \return if found, return.valid() is true, otherwise return.valid() is false
            /// \see locateNamespace findRealType findType
			/// \note if n contains version, return required type, else return current type
			Type locateType(const file_path& n) const;

			Type locateRealType(const file_path& n, const version_type& ver) const;

            /// locate the namespace with given name. the name can be a name path
            /// \param n name path. assume dim is '.', then
            ///  .xxx.yyy : search namespace xxx in root namespace, if found, search namespace yyy in namespace xxx, if found return the result.
            ///             otherwise returns a !valid() namespace. the starting dim means absolute name and search from root.
            ///  yyy      : search namespace yyy in current namespace, if found, return. if not found, search in the parent namespace, parent of parent namespace until root,
            ///             if found return immediately. if not found, return a !valid() namespace.
            ///  xxx.yyy  : search namespace xxx in the same way as above yyy. then search namespace yyy in the namespace xxx.
            /// \param separator of name path, suggestion is '.' or '/'
            /// \pre valid()
            /// \return if found, return.valid() is true, otherwise return.valid() is false
            /// \see locateType findNamespace
			Namespace locateNamespace(const file_path& n) const;


            /// find the type in current namespace by given name
            /// \pre valid()
            /// \return return.valid() if found
			/// \note it t has version part, return required type, else return current type
			Type findRealType (const file_path & t) const;

			Type findRealType (const file_path & t, const version_type& ver) const;

            /// find the type or type alias in current namespace by given name
            /// there are no same name type and alias in the same namespace
            /// \pre valid()
            /// \return return.valid() if found
			/// \note it t has version part, return required type, else return current type
			Type findType (const file_path & t) const;

			Type findType (const file_path & t, const version_type& ver) const;

            /// get the child namespace by given name
            /// \pre valid()
            /// \return return.valid() if found
			Namespace findNamespace (const file_path & ns) const;

            /// get the type alias by given name
            /// \pre valid()
            /// \return return.valid() if found
			TypeAlias findAlias(const file_path& t) const;

            /// get the types in current namespace
            /// \pre valid()
            /// \return return.valid() if found
            /// usage:
            ///     TypeRange r = ns.types();
            ///     for (TypeRange::iterator itr = r.begin(); itr != r.end(); ++itr)
            ///     { bala bala ... }
            /// !!!
			TypeSynonymRange types () const;

			TypeRange types (const file_path& n) const;

			TypeSynonym getTypeSynonym(const file_path& n) const;


            /// get the sub-namespace in current namespace
            /// \pre valid()
            /// \return return.valid() if found
            /// usage:
            ///     NamespaceRange r = ns.namespaces();
            ///     for (NamespaceRange::iterator itr = r.begin(); itr != r.end(); ++itr)
            ///     { bala bala ... }
            ///
			NamespaceRange namespaces() const;

            /// get the type alias in current namespace
            /// \pre valid()
            /// \return return.valid() if found
            /// usage:
            ///     TypeAliasRange r = ns.alias();
            ///     for (TypeAliasRange::iterator itr = r.begin(); itr != r.end(); ++itr)
            ///     { bala bala ... }
            ///
			TypeAliasRange alias () const;

            /// get the objects in current namespace
            /// \pre valid()
            /// \return return.valid() if found
            /// usage:
            ///     ObjectRange r = ns.objects();
            ///     for (ObjectRange::iterator itr = r.begin(); itr != r.end(); ++itr)
            ///     { bala bala ... }
            ///
			ObjectRange objects () const;

            /// find the object by given name and last version in current namespace
            /// \pram name
            /// \pre valid()
            /// \post NameValuePair.name == null || *NameValuePair.name == name
            /// \post if NameValuePair.name != null then NameValuePair.value != null
            /// \return if found NameValuePair.name != null
            NameValuePair findObject(const file_path& name) const;

            /// find the object by given name in current namespace
            /// \pram version empty means latest version
            /// \pre valid()
            /// \post NameValuePair.name == null || *NameValuePair.name == name
            /// \post if NameValuePair.name != null then NameValuePair.value != null
            /// \return if found NameValuePair.name != null
            NameValuePair findObject(const file_path& name, const version_type& version) const;

            /// return true if this namespace doesn't contain any type, type alias, and sub-namespace
            /// \pre valid()
			bool empty () const;

            /// get the parent namespace
            /// if return.valid() is false, means this namespace is a root namespace
            /// \pre valid()
			Namespace parent () const;


            /// get the root namespace which contains this namespace tree
            /// \pre valid()
			Namespace root() const;


            /// compare the internal address only, do not compare elements.
            /// return 0 if this and other refer to same namespace
			int compare (Namespace other) const;

            /// return true if it has be constrcted by not null imp ptointer
			bool valid() const;

            /// return true if valid()
			operator bool () const;

		private:
			NamespaceImp * m_imp;
            friend class ImpAccessor<NamespaceImp>;
	};
	DEFINE_COMPARE (Namespace);
    
    struct FindNamespaceResult{
        Namespace match;
        file_path rest;
    };
    
    XR_API extern FindNamespaceResult findNamespace(Namespace from, sub_file_path path);
    XR_API extern Namespace createNamespaceRecursively(Namespace from, sub_file_path path);
    XR_API extern Namespace createOrFindNamespaceRecursively(Namespace from, sub_file_path path);

    XR_EXCEPTION_TYPE(invalid_namespace_path);
    XR_EXCEPTION_TYPE(conflict_namespace_child_name);
    /// Builder pattern, so we can build a complicated namespace in tracaction
	class XR_API NamespaceBuilder
	{
	public:
        /// ctor
		NamespaceBuilder();

        /// dtor
		~NamespaceBuilder();

        /// \set name of this namespace
        /// \return this
		NamespaceBuilder& name(const file_path& n);

        /// create namespace recursively by given path
        /// if the a namespace exist and has the same name as the token in the path, it'll use the existed one.
        /// \thow invalid_namespace_path thow if path contains empty token, so start with dim is invalid.
        /// \exception basic
        NamespaceBuilder& createChild(const file_path& path);

        /// move the type from tb to current namespace, if success, tb will be reset
        /// \pre !tb.getName().empty()
        /// \pre !get().findType(tb.getName()).valid()
        /// \return this
		NamespaceBuilder& adopt(TypeBuilder& tb);

        /// move the namespace from b to current namespace as a child, if success, b will be reset
        /// \pre !b.getName().empty()
        /// \pre !get().findNamespace(tb.getName()).valid()
        /// \return this
		NamespaceBuilder& adopt(NamespaceBuilder& b);

        /// move the type alias from b to current namespace , if success, b will be reset
        /// \pre !b.getName().empty()
        /// \pre !get().findType(tb.getName()).valid()
        /// \return this
		NamespaceBuilder& adopt(TypeAliasBuilder& b);

        /// move the namespace from this builder to ns as child, if success, this builder  will be reset
        /// \pre !getName().empty()
        /// \pre !getParent().valid()
        /// \pre !ns.findNamespace(getName()).valid()
        /// \return namespace under construction
		Namespace adoptBy(Namespace ns);

        /// move all children of top namespace from builder into ns, then reset the top namespace
        /// this method doesn't affact getName() and getParent()
        /// \post get() return a clean namespace without any children including sub-namespace, type, alias orobject
		/// \note it requires the target ns is empty (no types, type alias, objects and children)
        NamespaceBuilder& adoptChildrenBy(Namespace ns);

        /// set the parent of current namespace
        /// \note this method doesn't check name confilt in the parent immediately
        NamespaceBuilder& parent(Namespace ns);

        /// move the namespace from this builder to parent as child, if success, this builder  will be reset
        /// \pre !getName().empty()
        /// \pre getParent().valid()
        /// \pre !getParent().findNamespace(getName()).valid()
        /// \return namespace under construction
        Namespace adoptBy();

        /// get the namespace under construction
        /// \post get().valid()
        Namespace get() const;

        /// get the namespace name
        const file_path& getName() const;

        /// get the parent namespace set by parent()
        Namespace getParent() const;

        /// \throw nothrow
        void swap(NamespaceBuilder& other);
	private:
        unique_ptr<NamespaceImp> m_imp;

        NamespaceBuilder(const NamespaceBuilder&); //diable copy ctor
        NamespaceBuilder& operator=(const NamespaceBuilder&); //disable assignment
	};

	inline Type getType(Namespace ns, const file_path& p){
		auto parent = ns.locateNamespace(p.parent());
		if (parent.valid())
			return parent.findType(p.filename());
		return Type();
	}
}}
#endif				//end XIRANG2_NAMESPACE_H__

