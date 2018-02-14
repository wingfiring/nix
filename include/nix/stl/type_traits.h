#ifndef NIX_STL_TYPE_TRAITS_H_
#define NIX_STL_TYPE_TRAITS_H_
#include <nix/config.h>
#include <type_traits>

namespace nix {
	// remove const voliatile and reference, backport from c++20
	template< class T > struct remove_cvref {
		typedef std::remove_cv_t<std::remove_reference_t<T>> type;
	};
	template< class T > using remove_cvref_t = typename remove_cvref<T>::type;
}

#endif //end NIX_STL_TYPE_TRAITS_H_
