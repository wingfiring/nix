//NIX_LICENSE_PLACE_HOLDER

#ifndef NIX_COMMON_RANGE_H__
#define NIX_COMMON_RANGE_H__

#include <nix/config.h>
#include <nix/operators.h>
#include <nix/contract.h>

//STL
#include <algorithm>
#include <iterator>

namespace nix
{
	template<typename Iterator>
	class range : totally_ordered<range<Iterator>>
	{
	public:
		typedef Iterator iterator;
		typedef std::size_t size_type;

		constexpr range() noexcept : m_beg(), m_end() {}

		template<typename OtherItr>
			explicit constexpr range(const OtherItr& first, const OtherItr& last) noexcept
			: m_beg(first), m_end(last) {}

		template<typename Cont> explicit range(Cont&& ) = delete;

		template<typename Cont> 
			explicit constexpr range(Cont& cont) noexcept
			: m_beg(std::begin(cont)), m_end(std::end(cont))
			{}

		template<typename Cont> 
			explicit constexpr range(const Cont& cont) noexcept
			: m_beg(std::cbegin(cont)), m_end(std::cend(cont))
			{}

		template<typename UItr> 
			constexpr range(const range<UItr>& cont) noexcept
			: m_beg(cont.begin()), m_end(cont.end())
			{}

		constexpr iterator begin() const noexcept { return m_beg;}
		constexpr iterator end() const noexcept { return m_end;}

		constexpr bool empty() const noexcept { return begin() == end();}
		constexpr size_type size() const noexcept { return std::distance(begin(), end());}

		constexpr explicit operator bool() const noexcept{ return begin() != end();}

		constexpr void swap(range& rhs) noexcept
		{
			using std::swap;
			swap(m_beg, rhs.m_beg);
			swap(m_end, rhs.m_end);
		}

		constexpr void remove_prefix(std::ptrdiff_t n) noexcept {
			NIX_EXPECTS(n >=0 && std::size_t(n) <= size());
			std::advance(m_beg, n);
		}
		constexpr void remove_suffix(std::ptrdiff_t n) noexcept {
			NIX_EXPECTS(n >=0 && std::size_t(n) <= size());
			std::advance(m_end, -n);
		}

		constexpr bool operator<(const range<Iterator>& rhs){
			return m_beg < rhs.m_beg
				|| (m_beg == rhs.m_beg && m_end < rhs.m_end);
		}
		constexpr bool operator==(const range<Iterator>& rhs){
			return m_beg == rhs.m_beg && m_end == rhs.m_end;
		}
	protected:
		Iterator m_beg, m_end;
	};

	template<typename Iter>
		constexpr void swap(range<Iter>& lhs, range<Iter>& rhs) noexcept 
		{
			lhs.swap(rhs);
		}

	template<typename Iter>
		constexpr range<Iter> make_range(Iter  first, Iter  last) noexcept
		{
			return range<Iter>(first, last);
		}

	template<typename Cont>
		constexpr auto to_range(Cont& cont) noexcept 
		-> decltype(make_range(std::begin(cont), std::end(cont)))
		{
			return make_range(std::begin(cont), std::end(cont));
		}

	template<typename Cont>
		constexpr auto to_crange(const Cont& cont) noexcept 
		-> decltype(make_range(std::cbegin(cont), std::cend(cont)))
		{
			return make_range(std::cbegin(cont), std::cend(cont));
		}

	template<typename Range1, typename Range2>
		constexpr bool lexicographical_compare( const Range1& lhs, const Range2& rhs) noexcept
		{
			return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
		}
}
#endif //end NIX_COMMON_RANGE_H__

