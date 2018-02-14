#ifndef NIX_UTILITY_INDIRECT_COMPARE_H_
#define NIX_UTILITY_INDIRECT_COMPARE_H_
#include <nix/config.h>
#include <functional>
#include <iterator>
#include <nix/stl/type_traits.h>
namespace nix{

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

	template<typename Itr> constexpr std::reverse_iterator<remove_cvref_t<Itr>> make_reverse_iterator(Itr&& itr){
		return std::reverse_iterator<remove_cvref_t<Itr>>(std::forward<Itr>(itr));
	}

}
#endif // end NIX_UTILITY_INDIRECT_COMPARE_H_

