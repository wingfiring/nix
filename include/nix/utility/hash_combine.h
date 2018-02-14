#ifndef NIX_UTILITY_HASH_COMBINE_H_
#define NIX_UTILITY_HASH_COMBINE_H_
#include <nix/config.h>
#include <functional>
namespace nix {
	// from boost::hash_combine
	template <class T> inline constexpr void hash_combine(std::size_t& seed, const T& v) noexcept
	{
		std::hash<T> hasher;
		seed ^= hasher(v) + 0x9e3779b9 + (seed<<6) + (seed>>2);
	}

}
#endif //end NIX_UTILITY_HASH_COMBINE_H_
