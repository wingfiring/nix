#ifndef NIX_UTILITY_MPL_H
#define NIX_UTILITY_MPL_H
namespace nix{
	namespace mpl{
		template<bool C, typename F1, typename F2> struct if_c {
			typedef F1 type;
		};
		template<typename F1, typename F2> struct if_c<false, F1, F2> {
			typedef F2 type;
		};
		template< typename  C, typename F1, typename F2 >
			struct if_ : detail::if_c<C::value, F1, F2> { };
		
		template< typename  C, typename F1, typename F2 >
			struct eval_if : if_<C,F1,F2>::type {};

		template< typename F1, typename F2 >
			struct or_ : std::bool_constant<bool, F1::value || F2::value>{};

		template< typename F1, typename F2 >
			struct and_ : std::bool_constant<bool, F1::value && F2::value>{};

		template<typename T> struct identity{
			typedef T type;
		};
	}
	template<class From, class To>
		struct is_convertible : std::is_convertible<From, To>{};

}
#endif // end NIX_UTILITY_MPL_H
