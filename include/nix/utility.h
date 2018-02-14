#ifndef NIX_UTILITY_H_
#define NIX_UTILITY_H_
#include <nix/config.h>
#include <nix/utility/no_copy.h>
#include <nix/utility/indirect_compare.h>

namespace nix{
	/// use to surppress unused variable warning.
	template<typename ...T> constexpr void unused(const T&... ) {};


	/// tag for object uninitialized
	struct no_initialize{};

	/// incomplete type
	struct null_type;

	/// 
	struct empty_type{};

	template<typename T> void checked_delete(T* p){
		static_assert(sizeof(T) > 0, "Must not delete an incomplete type");
		delete p;
	}

}

#endif //end NIX_UTILITY_H_
