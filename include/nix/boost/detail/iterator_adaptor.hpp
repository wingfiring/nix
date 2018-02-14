// (C) Copyright David Abrahams 2002.
// (C) Copyright Jeremy Siek    2002.
// (C) Copyright Thomas Witt    2002.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#ifndef NIX_BOOST_ITERATOR_ADAPTOR_H_
#define NIX_BOOST_ITERATOR_ADAPTOR_H_

#include <nix/utility.h>
#include <nix/utility/mpl.h>
#include <nix/iterator/iterator_traits.h>
#include <nix/iterator/iterator_facade.h>

namespace nix {

	using use_default = null_type;

	template<class To>
		struct is_convertible<use_default,To> : std::false_type{};

	template<typename From, typename To>
		struct enable_if_convertible
		: std::enable_if<is_convertible<From, To>::value>
		{};

	namespace detail
	{
		// If T is use_default, return the result of invoking
		// DefaultNullaryFn, otherwise return T.
		template <class T, class DefaultNullaryFn>
			struct ia_dflt_help
			: mpl::eval_if<
			  std::is_same<T, use_default>
			  , DefaultNullaryFn
			  , mpl::identity<T>
			  >
		{
		};

		// A metafunction which computes an iterator_adaptor's base class,
		// a specialization of iterator_facade.
		template <
			class Derived
			, class Base
			, class Value
			, class Traversal
			, class Reference
			, class Difference
			>
			struct iterator_adaptor_base
			{
				typedef iterator_facade<
					Derived

					, typename ia_dflt_help<
					Value
					, mpl::eval_if<
					std::is_same<Reference,use_default>
					, iterator_value<Base>
					, std::remove_reference<Reference>
					>
					>::type

					, typename ia_dflt_help<
					Traversal
					, iterator_traversal<Base>
					>::type

					, typename ia_dflt_help<
					Reference
					, mpl::eval_if<
					std::is_same<Value,use_default>
					, iterator_reference<Base>
					, std::add_reference<Value>
					>
					>::type

					, typename ia_dflt_help<
					Difference, iterator_difference<Base>
					>::type
					>
					type;
			};

		// workaround for aC++ CR JAGaf33512
		template <class Tr1, class Tr2>
			inline void iterator_adaptor_assert_traversal ()
			{
				BOOST_STATIC_ASSERT((is_convertible<Tr1, Tr2>::value));
			}
	}

	//
	// Iterator Adaptor
	//
	// The parameter ordering changed slightly with respect to former
	// versions of iterator_adaptor The idea is that when the user needs
	// to fiddle with the reference type it is highly likely that the
	// iterator category has to be adjusted as well.  Any of the
	// following four template arguments may be ommitted or explicitly
	// replaced by use_default.
	//
	//   Value - if supplied, the value_type of the resulting iterator, unless
	//      const. If const, a conforming compiler strips constness for the
	//      value_type. If not supplied, iterator_traits<Base>::value_type is used
	//
	//   Category - the traversal category of the resulting iterator. If not
	//      supplied, iterator_traversal<Base>::type is used.
	//
	//   Reference - the reference type of the resulting iterator, and in
	//      particular, the result type of operator*(). If not supplied but
	//      Value is supplied, Value& is used. Otherwise
	//      iterator_traits<Base>::reference is used.
	//
	//   Difference - the difference_type of the resulting iterator. If not
	//      supplied, iterator_traits<Base>::difference_type is used.
	//
	template <
		class Derived
		, class Base
		, class Value        = use_default
		, class Traversal    = use_default
		, class Reference    = use_default
		, class Difference   = use_default
		>
		class iterator_adaptor
		: public detail::iterator_adaptor_base<
		  Derived, Base, Value, Traversal, Reference, Difference
		  >::type
	{
		friend class iterator_core_access;

		protected:
		typedef typename detail::iterator_adaptor_base<
			Derived, Base, Value, Traversal, Reference, Difference
			>::type super_t;
		public:
		iterator_adaptor() {}

		explicit iterator_adaptor(Base const &iter)
			: m_iterator(iter)
		{
		}

		typedef Base base_type;

		Base const& base() const
		{ return m_iterator; }

		protected:
		// for convenience in derived classes
		typedef iterator_adaptor<Derived,Base,Value,Traversal,Reference,Difference> iterator_adaptor_;

		//
		// lvalue access to the Base object for Derived
		//
		Base const& base_reference() const
		{ return m_iterator; }

		Base& base_reference()
		{ return m_iterator; }

		private:
		//
		// Core iterator interface for iterator_facade.  This is private
		// to prevent temptation for Derived classes to use it, which
		// will often result in an error.  Derived classes should use
		// base_reference(), above, to get direct access to m_iterator.
		//
		typename super_t::reference dereference() const
		{ return *m_iterator; }

		template <
			class OtherDerived, class OtherIterator, class V, class C, class R, class D
			>
			bool equal(iterator_adaptor<OtherDerived, OtherIterator, V, C, R, D> const& x) const
			{
				// Maybe readd with same_distance
				//           BOOST_STATIC_ASSERT(
				//               (detail::same_category_and_difference<Derived,OtherDerived>::value)
				//               );
				return m_iterator == x.base();
			}

		typedef typename iterator_category_to_traversal<
			typename super_t::iterator_category
			>::type my_traversal;

# define NIX_BOOST_ITERATOR_ADAPTOR_ASSERT_TRAVERSAL(cat) \
		detail::iterator_adaptor_assert_traversal<my_traversal, cat>();

		void advance(typename super_t::difference_type n)
		{
			NIX_BOOST_ITERATOR_ADAPTOR_ASSERT_TRAVERSAL(random_access_traversal_tag)
				m_iterator += n;
		}

		void increment() { ++m_iterator; }

		void decrement()
		{
			NIX_BOOST_ITERATOR_ADAPTOR_ASSERT_TRAVERSAL(bidirectional_traversal_tag)
				--m_iterator;
		}

		template <
			class OtherDerived, class OtherIterator, class V, class C, class R, class D
			>
			typename super_t::difference_type distance_to(
					iterator_adaptor<OtherDerived, OtherIterator, V, C, R, D> const& y) const
			{
				NIX_BOOST_ITERATOR_ADAPTOR_ASSERT_TRAVERSAL(random_access_traversal_tag)
					// Maybe readd with same_distance
					//           BOOST_STATIC_ASSERT(
					//               (detail::same_category_and_difference<Derived,OtherDerived>::value)
					//               );
					return y.base() - m_iterator;
			}

# undef NIX_BOOST_ITERATOR_ADAPTOR_ASSERT_TRAVERSAL

		private: // data members
		Base m_iterator;
	};
} // namespace nix


#endif // NIX_BOOST_ITERATOR_ADAPTOR_H_
