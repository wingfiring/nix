#ifndef NIX_OFFSET_RANGE_H_
#define NIX_OFFSET_RANGE_H_
#include <nix/config.h>
#include <nix/contract.h>
#include <nix/utility/hash_combine.h>

#include <cstdint>
namespace nix{
	/// 64 bits long size_t type
	using long_size_t =  std::uint64_t;
	/// 64 bits long offset type
	using long_offset_t = std::int64_t;

	/// a close/open section of long_offset_t to represent content range
	class offset_range {
		long_offset_t m_begin = 0;
		long_offset_t m_end = 0;
	public:
		/// \pre b <= e
		constexpr explicit offset_range(long_offset_t b, long_offset_t e) noexcept
			: m_begin(b), m_end(e){
				NIX_EXPECTS(b <= e);
			}

		/// the default begin and end are zero
		constexpr offset_range() noexcept : offset_range(0,0){}

		/// \return begin offset
		constexpr long_offset_t begin() const noexcept { return m_begin;}
		
		/// \return end offset
		constexpr long_offset_t end() const noexcept { return m_end;}

		/// \return end() - begin()
		constexpr long_size_t size() const noexcept { return m_end - m_begin;}

		/// \return end() == begin()
		constexpr bool empty() const noexcept { return m_begin == m_end; }

		/// \return !empty()
		constexpr explicit operator bool() const noexcept { return !empty(); }

		/// \return true if rhs contains b.
		constexpr bool in(offset_range rhs) const noexcept{
			return begin() >= rhs.begin() && end() <= rhs.end();
		}

		/// \return true if contains b. 
		/// \note if a equals to b, then a contains b and b contains a.
		constexpr bool contains(offset_range rhs) const noexcept{
			return rhs.in(*this);
		}
		/// \return true if begin of rhs in this range
		constexpr bool cross_left(offset_range rhs) const noexcept{
			return begin() < rhs.begin()
				&& end() > rhs.begin();
		}
		/// \return true if end of rhs in this range
		constexpr bool cross_right(offset_range rhs) const noexcept{
			return begin() < rhs.end()
				&& end() > rhs.end();
		}
		/// \return end() == rhs.begin()
		constexpr bool adjoining(offset_range rhs) const noexcept {
			return end() == rhs.begin();
		}
	};

	/// \return lhs.begin() == rhs.begin() && lhs.end() == rhs.end();
	constexpr bool operator==(offset_range lhs, offset_range rhs) noexcept {
		return lhs.begin() == rhs.begin() && lhs.end() == rhs.end();
	}
	/// \return !(lhs == rhs);
	constexpr bool operator!=(offset_range lhs, offset_range rhs) noexcept {
		return !(lhs == rhs);
	}
	/// \return lhs.begin() < rhs.begin() || (lhs.begin() == rhs.begin() && lhs.end() < rhs.end());
	constexpr bool operator<(offset_range lhs, offset_range rhs) noexcept {
		return lhs.begin() < rhs.begin() 
			|| (lhs.begin() == rhs.begin() && lhs.end() < rhs.end());
	}
	/// \return rhs < lhs;
	constexpr bool operator>(offset_range lhs, offset_range rhs) noexcept {
		return rhs < lhs;
	}
	/// \return !(rhs < lhs)
	constexpr bool operator<=(offset_range lhs, offset_range rhs) noexcept {
		return !(rhs < lhs);
	}
	/// \return !(lhs < rhs)
	constexpr bool operator>=(offset_range lhs, offset_range rhs) noexcept {
		return !(lhs < rhs);
	}
}
namespace std{
	template<> struct hash<nix::offset_range> 
	{
		typedef nix::offset_range argument_type;
		typedef std::size_t result_type;
		
		result_type operator()(argument_type s) const noexcept {
			auto ret = std::hash<decltype(s.begin())>()(s.begin());
			nix::hash_combine(ret, s.end());
			return ret;
		}

	};
}
#endif //NIX_OFFSET_RANGE_H_

