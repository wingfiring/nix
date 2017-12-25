#ifndef NIX_UTILITY_H_
#define NIX_UTILITY_H_
#include <nix/config.h>
#include <functional>
#include <iterator>
#include <type_traits>

namespace nix{
	/// use to surppress unused variable warning.
	template<typename ...T> constexpr void unused(const T&... ) {};

	/// tag for object uninitialized
	struct no_initialize{};

	/// indirect compare
	template<typename Compare>
	struct indirect_compare : Compare
	{
		template<typename Itr>
		constexpr bool operator()(const Itr& lhs, const Itr& rhs) const
		{
			return Compare::operator()(*lhs, *rhs);
		}
	};
	using indirect_equal_to = indirect_compare<std::equal_to<>>;
	using indirect_not_equal_to = indirect_compare<std::not_equal_to<>>;
	using indirect_greater = indirect_compare<std::greater<>>;
	using indirect_less = indirect_compare<std::less<>>;
	using indirect_greater_equal = indirect_compare<std::greater_equal<>>;
	using indirect_less_equal = indirect_compare<std::less_equal<>>;

	template<typename Itr> constexpr std::reverse_iterator<Itr> make_reverse_iterator(Itr itr){
		return std::reverse_iterator<Itr>(std::move(itr));
	}

	/// incomplete type
	struct null_type;

	/// 
	struct empty_type{};

	template<typename T> void check_delete(T* p){
		static_assert(sizeof(T) > 0, "Must not delete an incomplete type");
		delete p;
	}

}

#endif //end NIX_UTILITY_H_
