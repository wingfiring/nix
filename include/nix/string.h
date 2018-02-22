//NIX_LICENSE_PLACE_HOLDER

#ifndef NIX_STRING_H
#define NIX_STRING_H

#include <nix/config.h>
#include <nix/memory.h>
#include <nix/string/shared_data.h>
#include <nix/operators.h>

//STL
#include <iosfwd>
#include <cstdint>
#include <string>		//introduce char_traits and stream

namespace nix
{
	namespace detail{
		template<typename T>
			constexpr inline size_t hash_str(T const * s, size_t size) noexcept{
				size_t hash = 2166136261U;
				size_t count = size * sizeof(T) / sizeof(uint_fast32_t);
				size_t mod = (size * sizeof(T) % sizeof(uint_fast32_t)) / sizeof(T);
				auto int_p = static_cast<const uint_fast32_t*>(static_cast<const void*>(s));

				for (auto int_end = int_p + count; int_p < int_end; ++int_p)
					hash = 16777619U * hash ^ size_t(*int_p);
				auto char_p = static_cast<const T*>(static_cast<const void*>(int_p));
				for (auto char_end = char_p + mod; char_p < char_end; ++char_p)
					hash = 16777619U * hash ^ size_t(*char_p);
				return hash;
			}

		// copy from /usr/include/c++/7/bits/ostream_insert.h
		template<typename CharT, typename Traits>
			inline void ostream_write(std::basic_ostream<CharT, Traits>& out,
					const CharT* s, std::streamsize n)
			{
				typedef std::basic_ostream<CharT, Traits>       ostream_type;
				typedef typename ostream_type::ios_base    ios_base;

				const auto put = out.rdbuf()->sputn(s, n);
				if (put != n)
					out.setstate(ios_base::badbit);
			}

		template<typename CharT, typename Traits>
			inline void ostream_fill(std::basic_ostream<CharT, Traits>& out, std::streamsize n)
			{
				typedef std::basic_ostream<CharT, Traits>       ostream_type;
				typedef typename ostream_type::ios_base    ios_base;

				const CharT c = out.fill();
				for (; n > 0; --n)
				{
					const auto put = out.rdbuf()->sputc(c);
					if (Traits::eq_int_type(put, Traits::eof()))
					{
						out.setstate(ios_base::badbit);
						break;
					}
				}
			}

		template<typename CharT, typename Traits>
			std::basic_ostream<CharT, Traits>& ostream_insert(std::basic_ostream<CharT, Traits>& out,
					const CharT* s, std::streamsize n)
			{
				typedef std::basic_ostream<CharT, Traits>       ostream_type;
				typedef typename ostream_type::ios_base    ios_base;

				typename ostream_type::sentry cerb(out);
				if (cerb)
				{
					try
					{
						const auto w = out.width();
						if (w > n)
						{
							const bool left = ((out.flags()
										& ios_base::adjustfield)
									== ios_base::left);
							if (!left)
								ostream_fill(out, w - n);
							if (out.good())
								ostream_write(out, s, n);
							if (left && out.good())
								ostream_fill(out, w - n);
						}
						else
							ostream_write(out, s, n);
						out.width(0);
					}
					catch(...)
					{ out.setstate(ios_base::badbit); }
				}
				return out;
			}
	}
	template<typename CharT>
		struct char_traits : std::char_traits<std::remove_cv_t<CharT>> {
			using base_type = std::char_traits<std::remove_cv_t<CharT>>;
			using char_type = typename base_type::char_type;
			using size_type = std::size_t;

			static constexpr int traits_compare(const char_type * left, size_type left_size,
					const char_type * right, size_type right_size) noexcept
			{
				auto min_size = std::min(left_size,  right_size);
				auto ans = base_type::compare(left, right, min_size);

				if (ans != 0) return (ans);

				if (left_size < right_size) return (-1);

				if (left_size > right_size) return (1);

				return 0;
			}
		};

	template<typename T, typename U>
		struct concator;

	template<typename T> struct is_concator : std::false_type{ };
	template<typename T, typename U> struct is_concator<concator<T, U>> : std::true_type{ };


	//this class intends to hold literal string
	template< typename CharT >
		class basic_range_string : totally_ordered<basic_range_string<CharT>>
	{
		public:
			typedef char_traits<CharT> traits_type;	//std::basic_string comptible
			typedef typename traits_type::char_type char_type;

			typedef CharT value_type;

			typedef std::size_t size_type;
			typedef std::ptrdiff_t difference_type;

			typedef value_type& reference;
			typedef value_type* pointer;
			typedef pointer iterator;

			typedef const char_type& const_reference;
			typedef const char_type* const_pointer;
			typedef const_pointer const_iterator;

			static const size_type npos = size_type(-1);

			/// \ctor
			/// \post empty()
			constexpr basic_range_string() noexcept: m_beg(0), m_end(0){}

			constexpr basic_range_string(pointer first, pointer last) noexcept
				: m_beg(first), m_end(last)
				{
					NIX_EXPECTS(first <= last);
					NIX_EXPECTS(first == last || first != nullptr);
				}

			// convert non-const to const
			template<typename UCharT>
				constexpr basic_range_string (basic_range_string<UCharT> rhs) noexcept
				: basic_range_string(rhs.begin(), rhs.end())
				{}


			/// \ctor
			constexpr explicit basic_range_string(pointer str) noexcept
				: basic_range_string(str, str + traits_type::length(str))
				{
					NIX_EXPECTS(str != 0);
				}

			/// \ctor
			/// \pre  size == 0 || str != 0
			constexpr explicit basic_range_string(pointer str, size_type size) noexcept
				: basic_range_string(str, str + size)
				{
				}

			constexpr explicit basic_range_string(const std::basic_string<std::remove_const_t<CharT>>& src) noexcept
				: basic_range_string(src.data(), src.size())
				{
				}
			constexpr explicit basic_range_string(std::basic_string<std::remove_const_t<CharT>>& src) noexcept
				: basic_range_string(src.data(), src.size())
				{
				}

			/// \return true if size() == 0
			constexpr bool empty() const noexcept{ return m_beg == m_end; }

			constexpr size_type size() const noexcept{ return m_end - m_beg;}

			constexpr iterator begin() const noexcept{ return m_beg;}
			constexpr iterator end() const noexcept{ return m_end;}

			/// \return never return null
			constexpr pointer data() const noexcept{ return  m_beg;}

			/// there is no non-const version
			constexpr reference operator[] (size_type index) const noexcept{
				NIX_EXPECTS(index >= 0 && index < size());
				return m_beg[index];
			}

			constexpr void swap(basic_range_string& rhs) noexcept
			{
				std::swap(m_beg, rhs.m_beg);
				std::swap(m_end, rhs.m_end);
			}

			constexpr void remove_prefix(size_type n) noexcept{
				NIX_EXPECTS(n <= size());
				m_beg += n;
			}
			constexpr void remove_suffix(size_type n) noexcept{
				NIX_EXPECTS(n <= size());
				m_end -= n;
			}
		private:
			pointer m_beg;
			pointer m_end;
	};

	template<typename CharT>
		constexpr void swap(basic_range_string<CharT>& lhs, basic_range_string<CharT>& rhs) noexcept
		{
			lhs.swap(rhs);
		}

	template<typename CharT, typename CharU>
		constexpr bool operator < (basic_range_string<CharT> lhs
				, basic_range_string<CharU> rhs) noexcept {
			typedef typename basic_range_string<CharT>::traits_type char_traits;
			return char_traits::traits_compare(
					lhs.data(), lhs.size(), rhs.data(), rhs.size()) < 0;
		}

	template<typename CharT, typename CharU>
		constexpr bool operator == (basic_range_string<CharT> lhs
				, basic_range_string<CharU> rhs)
		{
			typedef typename basic_range_string<CharT>::traits_type char_traits;
			return char_traits::traits_compare(
					lhs.data(), lhs.size(), rhs.data(), rhs.size()) == 0;
		}

	template<typename CharT, std::size_t N>
		constexpr basic_range_string<CharT> literal(CharT(&src) [N]) noexcept{
			return basic_range_string<CharT>(src, N-1);
		}

	template<typename CharT, std::size_t N>
		constexpr basic_range_string<const CharT> literal(const CharT(&src) [N]) noexcept{
			return basic_range_string<const CharT>(src, N-1);
		}

	template<typename CharT>
		constexpr basic_range_string<CharT> as_range_string(CharT* src) noexcept{
			NIX_EXPECTS(src);
			return basic_range_string<CharT>(src);
		}

	template<typename CharT>
		constexpr basic_range_string<const CharT> as_range_string(const CharT* src) noexcept{
			NIX_EXPECTS(src);
			return basic_range_string<const CharT>(src);
		}

		constexpr basic_range_string<const char> operator "" _str(const char* str, size_t len) noexcept{
			NIX_EXPECTS(str);
			return  basic_range_string<const char>(str, str + len);
		}
		constexpr basic_range_string<const wchar_t> operator "" _str(const wchar_t* str, size_t len) noexcept{
			NIX_EXPECTS(str);
			return  basic_range_string<const wchar_t>(str, str + len);
		}

	template <class CharT, class Traits>
		std::basic_ostream<CharT, Traits>&
		operator<<(std::basic_ostream<CharT, Traits>& os, 
				basic_range_string<CharT> v) {
			return detail::ostream_insert(os, v.data(), v.size());
		}

	typedef basic_range_string<const char> const_range_string;
	typedef basic_range_string<const wchar_t> const_wrange_string;
	typedef basic_range_string<char> range_string;
	typedef basic_range_string<wchar_t> wrange_string;

	///This class intends to be used to hold an immutable string
	template < typename CharT >
		class basic_string : totally_ordered<basic_string<CharT>>
	{
		public:
		typedef char_traits<CharT> traits_type;
		typedef CharT value_type;

		typedef heap heap_type;
		typedef std::size_t size_type;
		typedef std::ptrdiff_t difference_type;

		typedef const value_type& const_reference;
		typedef const_reference reference;
		typedef const value_type* const_pointer;
		typedef const_pointer pointer;

		typedef const_pointer iterator;
		typedef const_pointer const_iterator;

		static const size_type npos = size_type(-1);

		private:
		static constexpr size_t tiny_buffer_limit = 16;
		static constexpr const uint8_t big_string_flag = 0x80;
		static constexpr size_t char_data_limit = (tiny_buffer_limit / sizeof(CharT)) -1;

		typedef private_::shared_data<CharT> data_type;
		union{
			data_type* m_data;
			char tiny_buf[tiny_buffer_limit];
			CharT char_data[tiny_buffer_limit /sizeof(CharT)];
		};
		constexpr uint8_t tiny_size_() const noexcept {
			return uint8_t(tiny_buf[tiny_buffer_limit-1]);
		}
		constexpr void set_tiny_size_(size_t n) noexcept{
			NIX_EXPECTS(n < char_data_limit || n == big_string_flag);
			tiny_buf[tiny_buffer_limit-1] = char(uint8_t(n));
		}
		constexpr void init_clear_() noexcept{
			std::char_traits<char>::assign(tiny_buf, tiny_buffer_limit, 0);
		}

		constexpr void init_tiny_(const CharT* s, size_t n) noexcept {
			NIX_EXPECTS(n < char_data_limit);
			traits_type::copy(char_data, s, n);
			char_data[n] = CharT();
			set_tiny_size_(n);
		}

		static const_pointer empty_string() noexcept{
			static value_type empty_(0);
			return &empty_;
		}

		static data_type* new_data2(heap& hp, size_type n)
		{
			NIX_EXPECTS(n >= char_data_limit);
			data_type* p =  
				reinterpret_cast<data_type* >(hp.malloc(
							sizeof(data_type) +  sizeof(CharT) * n 
							, sizeof(std::size_t), 0));
			p->heap_ptr = &hp;
			p->counter.store(1, std::memory_order_relaxed);
			p->size = n;
			return p;
		}

		data_type* new_data(const_pointer s, size_type n){
			NIX_EXPECTS(s != nullptr && n >= char_data_limit);
			data_type* p = new_data2(get_heap(), n);
			traits_type::copy(p->data, s, n);
			p->data[n] = CharT();	// for range string, it is not a null-terminate string
			p->hash = detail::hash_str(p->data, n);
			return p;
		}

		struct internal_{};

		constexpr basic_string(data_type* buffer, internal_ ) noexcept
			: m_data(buffer)
			{
				set_tiny_size_(big_string_flag);
			}

		void add_ref_() noexcept{
			NIX_EXPECTS(!is_tiny());
			m_data->addref();
		}
		void release_ref_() noexcept{
			NIX_EXPECTS(!is_tiny());
			if(0 == m_data->release())
			{
				get_heap().free(m_data, sizeof(data_type) +  sizeof(CharT) * m_data->size, sizeof(std::size_t));
			}
			m_data = 0;
		}
		std::size_t tiny_hash_() const noexcept{
			return detail::hash_str(char_data, tiny_size_());
		}
		public:
		constexpr bool is_tiny() const noexcept {
			return tiny_size_() < char_data_limit;
		}
		constexpr basic_string() noexcept{
			init_clear_();
		}

		basic_string(basic_range_string<const CharT> src, heap& h = get_global_heap())
			: basic_string()
		{
			if (src.size() < char_data_limit)
				init_tiny_(src.data(), src.size());
			else {
				m_data = new_data(src.data(), src.size());
				set_tiny_size_(big_string_flag);
			}
		}

		template<size_t N>
			basic_string(const CharT (&src)[N], heap& h = get_global_heap())
			: basic_string(literal(src), h)
			{
			}

		basic_string(const_pointer src, heap& h = get_global_heap())
			: basic_string(basic_range_string<const CharT>(src), h)
		{
		}

		basic_string(basic_string&& rhs)
		{
			std::char_traits<char>::copy(tiny_buf, rhs.tiny_buf, sizeof(rhs.tiny_buf));
			if (!is_tiny())
				rhs.init_clear_();
		}

		basic_string(const basic_string& rhs)
		{
			std::char_traits<char>::copy(tiny_buf, rhs.tiny_buf, sizeof(rhs.tiny_buf));
			if (!is_tiny())
				add_ref_();
		}

		basic_string(const std::string& rhs, heap& h = get_global_heap())
			: basic_string(basic_range_string<const CharT>(rhs), h)
		{
		}

		template<typename Range, typename Enable = typename std::enable_if<
			!(is_concator<Range>::value || std::is_pointer<Range>::value)
			, void>::type>
			explicit basic_string(const Range& r, heap& h = get_global_heap())
			: basic_string() 
			{
				size_type len = std::distance(r.begin(), r.end());
				value_type* p = nullptr;
				if (len < char_data_limit) {
					p = char_data;
					set_tiny_size_(len);
				}
				else {
					m_data = new_data2(h, len);
					p = m_data->data;
					set_tiny_size_(big_string_flag);
				}
				p = std::copy(r.begin(), r.end(), p);

				*p = value_type();
				if (len >= char_data_limit)
					m_data->hash = detail::hash_str(m_data->data, len);
			}
		template<typename T, typename U> basic_string(const concator<T,U>& c, heap& h = get_global_heap())
			: basic_string()
		{
			size_type len = c.size();
			value_type* p = nullptr;
			if (len < char_data_limit) {
				p = char_data;
				set_tiny_size_(len);
			}
			else {
				m_data = new_data2(h, len);
				p = m_data->data;
				set_tiny_size_(big_string_flag);
			}

			c.copy_(p, &c);

			p[len] = value_type();
			if (len >= char_data_limit)
				m_data->hash = detail::hash_str(m_data->data, len);
		}

		basic_string& operator=(const basic_string& rhs)
		{
			if (this != &rhs)
				basic_string(rhs).swap(*this);
			return *this;
		}
		basic_string& operator=(basic_string&& rhs)
		{
			basic_string(std::move(rhs)).swap(*this);
			return *this;
		}
		basic_string& operator=(const basic_range_string<const CharT>& rhs)
		{
			basic_string(rhs, get_heap()).swap(*this);
			return *this;
		}
		template<typename Range, typename Enable = typename std::enable_if<!is_concator<Range>::value, void>::type>
			basic_string& operator=(const Range& r)
			{
				basic_string(r, get_heap()).swap(*this);
				return *this;
			}

		~basic_string() noexcept
		{
			if (!is_tiny())
			{
				release_ref_();
			}
		}

		operator basic_range_string<const CharT>() const noexcept{
			return range_str();
		}
		basic_range_string<const CharT> range_str() const noexcept{
			return basic_range_string<const CharT>(begin(), end());
		}

		void swap(basic_string& rhs) noexcept
		{
			char tmp [sizeof(tiny_buf)];
			std::char_traits<char>::copy(tmp, tiny_buf, sizeof(tiny_buf));
			std::char_traits<char>::copy(tiny_buf, rhs.tiny_buf, sizeof(tiny_buf));
			std::char_traits<char>::copy(rhs.tiny_buf, tmp, sizeof(tiny_buf));
		}

		void clear() noexcept
		{
			if (!is_tiny())
			{
				release_ref_();
			}
			set_tiny_size_(0);
		}

		bool empty() const noexcept{
			return tiny_size_() == 0;
		}

		size_type hash() const noexcept { 
			return is_tiny() ? tiny_hash_() : m_data->hash;
		}

		size_type size() const noexcept{ return is_tiny() ? tiny_size_() : m_data->size;}
		const_iterator begin() const noexcept{ return is_tiny() ? char_data : m_data->data;}
		const_iterator end() const noexcept { return begin() + size();}
		const_pointer c_str() const noexcept { return begin();}
		const_pointer data() const noexcept { return begin();}

		value_type operator[] (size_type index) const noexcept{
			NIX_EXPECTS(index >= 0 && index < size());
			return data()[index];
		}

		heap& get_heap() const  noexcept{
			return empty() ? get_global_heap() : *m_data->heap_ptr;
		}
	};
	typedef basic_string<char> string;
	typedef basic_string<wchar_t> wstring;

	template<typename CharU>
		struct hash_str{
			size_t operator()(const basic_string<CharU>& str) const{
				return str.hash();
			}
		};

	typedef hash_str<char> hash_string;
	typedef hash_str<wchar_t> hash_wstring;

	template<typename T> struct is_string : std::false_type{ };
	template<typename T> struct is_string<basic_range_string<T>> : std::true_type{ };
	template<typename T> struct is_string<basic_string<T>> : std::true_type{ };
	template<typename T> struct is_string<std::basic_string<T>> : std::true_type{ };

	template<typename T, typename U>
		struct concator{

			const T* left;
			const U* right;

			std::size_t size() const{
				return left->size() + right->size();
			}
			template<typename CharT, typename Range>
				static CharT* copy_(CharT* buf, const Range* s){
					std::copy(s->begin(), s->end(), buf);
					return buf + s->size();
				}
			template<typename CharT, typename CT, typename CU>
				static CharT* copy_(CharT* buf, const concator<CT, CU>* s){
					auto p = s->copy_(buf, s->left);
					return s->copy_(p, s->right);
				}
			explicit concator(const T& lhs, const U& rhs)
				: left(&lhs), right(&rhs)
			{}
			concator& operator=(const concator& ) = delete;
			concator(const concator& rhs) = delete;
		};

	template<typename Str1, typename Str2, typename = typename std::enable_if<is_string<Str1>::value && is_string<Str2>::value>::type>
		constexpr concator<Str1, Str2 > operator<< (const Str1& lhs, const Str2&  rhs) noexcept{
			return concator<Str1, Str2>(lhs, rhs);
		}
	template<typename CT, typename CU, typename Str, typename = typename std::enable_if<is_string<Str>::value>::type>
		constexpr concator<concator<CT, CU>, Str> operator<< (concator<CT, CU>&& lhs, const Str& rhs) noexcept{
			return concator<concator<CT,CU>, Str>(lhs, rhs);
		}
	template<typename Str, typename CT, typename CU, typename = typename std::enable_if<is_string<Str>::value, void>::type>
		constexpr concator<Str, concator<CT,CU> > operator<< (const Str& lhs, concator<CT, CU>&& rhs) noexcept{
			return concator<Str, concator<CT, CU> >(lhs, rhs);
		}
	template<typename CT1, typename CU1, typename CT2, typename CU2>
		constexpr concator<concator<CT1, CU1>, concator<CT2,CU2>> operator<< (concator<CT1, CU1>&& lhs, concator<CT2, CU2>&& rhs) noexcept{
			return concator<concator<CT1, CU1>, concator<CT2, CU2>>(lhs, rhs);
		}

	template<typename CharT>
		void swap(basic_string<CharT>& lhs, basic_string<CharT>& rhs)
		{
			lhs.swap(rhs);
		}

	template<typename CharT>
		bool operator < (const basic_string<CharT>& lhs, const basic_string<CharT>& rhs)
		{
			typedef basic_range_string<const CharT> range_type;
			return lhs.c_str() != rhs.c_str()
				&& static_cast<range_type>(lhs) < static_cast<range_type>(rhs);
		}
	template<typename CharT, typename CharU>
		bool operator < (const basic_string<CharT>& lhs, basic_range_string<CharU> rhs)
		{
			typedef basic_range_string<const CharT> range_type;
			return static_cast<range_type>(lhs) < rhs;
		}
	template<typename CharT, typename CharU>
		bool operator < (basic_range_string<CharT> lhs, const basic_string<CharU>& rhs)
		{
			typedef basic_range_string<const CharU> range_type;
			return lhs < static_cast<range_type>(rhs);
		}

	template<typename CharT>
		bool operator == (const basic_string<CharT>& lhs
				, const basic_string<CharT>& rhs)
		{
			typedef basic_range_string<const CharT> range_type;
			return lhs.c_str() == rhs.c_str()||
				( lhs.hash() == rhs.hash()
				  && static_cast<range_type>(lhs) == static_cast<range_type>(rhs));
		}

	template<typename CharT, typename CharU>
		bool operator == (const basic_string<CharT>& lhs, basic_range_string<CharU> rhs)
		{
			typedef basic_range_string<const CharT> range_type;
			return static_cast<range_type>(lhs) == rhs;
		}
	template<typename CharT, typename CharU>
		bool operator == (basic_range_string<CharT> lhs, const basic_string<CharU>& rhs)
		{
			typedef basic_range_string<const CharU> range_type;
			return lhs == static_cast<range_type>(rhs);
		}

	template<typename CharT>
		std::basic_ostream<CharT >& operator<<(
				std::basic_ostream<CharT >& os, const basic_string<CharT>& out)
		{
			return os << out.c_str();
		}

	template<typename CharT>
		std::basic_ostream<CharT >& operator>> (
				std::basic_ostream<CharT >& os, const basic_string<CharT>& in)
		{
			std::basic_string<CharT> tmp;
			os >> tmp;
			in = tmp.c_str();
		}

	template<typename CharT>
		std::basic_ostream<CharT >& operator<<(
				std::basic_ostream<CharT >& os, const basic_range_string<const CharT>& out)
		{
			return os << basic_string<CharT>(out);
		}
}
#endif //end NIX_STRING_H

