#ifndef NIX_COMMON_ENDIAN_H__
#define NIX_COMMON_ENDIAN_H__

#include <nix/config.h>

#ifdef LINUX_OS_
#  include <endian.h>
#elif defined (MACOS_OS_)
#  include <machine/endian.h>
#elif defined (WIN32_OS_)
#  ifndef __LITTLE_ENDIAN
#    define __LITTLE_ENDIAN 1234
#    define __BIG_ENDIAN    4321
#  endif // __LITTLE_ENDIAN
#  ifndef __BYTE_ORDER
#    define __BYTE_ORDER __LITTLE_ENDIAN
#  endif  // __BYTE_ORDER
#else
#  error "Unsupport OS"
#endif

#include <nix/endian/intrinsic.h>

namespace nix{
	struct little_endian_tag{};
	struct big_endian_tag{};
	using exchange_endian_tag = little_endian_tag;

#if (__BYTE_ORDER == __LITTLE_ENDIAN)
#  define NIX_LITTLE_ENDIAN
	using local_endian_tag = little_endian_tag;
#elif (__BYTE_ORDER == __BIG_ENDIAN)
#  define NIX_BIG_ENDIAN
	using local_endian_tag = big_endian_tag;
#elif (__BYTE_ORDER == __PDP_ENDIAN)
# error "PDP byte order is not supported"
#else
#  error Unknown machine endianness detected.
#endif

# define NIX_BYTE_ORDER __BYTE_ORDER

	namespace endian{
		template<std::size_t N> struct size_to_type;
		template<> struct size_to_type<1> { using type = uint8_t;};
		template<> struct size_to_type<2> { using type = uint16_t;};
		template<> struct size_to_type<4> { using type = uint32_t;};
		template<> struct size_to_type<8> { using type = uint64_t;};
		template<std::size_t N> using underling_type = typename size_to_type<N>::type;

		template<std::size_t N> struct byte_reverse;
		template<> struct byte_reverse<1> { 
			using type = underling_type<1>;
			static constexpr type apply(type t){return t;}
		};
		template<> struct byte_reverse<2> {
			using type = underling_type<2>;
			static constexpr type apply(type t){return NIX_ENDIAN_INTRINSIC_BYTE_SWAP_2(t);}
		};
		template<> struct byte_reverse<4> {
			using type = underling_type<4>;
			static constexpr type apply(type t){return NIX_ENDIAN_INTRINSIC_BYTE_SWAP_4(t);}
		};
		template<> struct byte_reverse<8> {
			using type = underling_type<8>;
			static constexpr type apply(type t){return NIX_ENDIAN_INTRINSIC_BYTE_SWAP_8(t);}
		};

		template<typename From, typename To, typename T> struct convert_imp{
			static constexpr T apply(T t){
				using reverser = byte_reverse<sizeof(T)>;
				using use_type = typename reverser::type;
				return T(reverser::apply(use_type(t)));
			}
		};

		template<typename From, typename T, std::size_t N> struct convert_imp<From, From, T, N>{
			static constexpr T apply(T t){
				return t;
			}
		};

		template<typename From, typename To, typename T> constexpr T convert(T t){
			return convert_imp<From, To, T>::apply(t);
		}
	}
	// local <--> exchange
	template<typename T> struct local2ex{
		static constexpr T operator()(T t) {
			return endian::convert<local_endian_tag, exchange_endian_tag>(t);
		}
	};
	template<typename T> struct ex2local{
		static constexpr T operator()(T t) {
			return endian::convert<exchange_endian_tag, local_endian_tag>(t);
		}
	};
	template<typename T> constexpr T local2ex_f( T t){
		return endian::convert<local_endian_tag, exchange_endian_tag>(t);
	}
	template<typename T> constexpr T ex2local_f( T t){
		return endian::convert<exchange_endian_tag, local_endian_tag>(t);
	}

	// little <--> big
	template<typename T> struct little2big{
		static constexpr T operator()(T t) {
			return endian::convert<little_endian_tag, big_endian_tag>(t);
		}
	};
	template<typename T> struct big2little{
		static constexpr T operator()(T t) {
			return endian::convert<big_endian_tag, little_endian_tag>(t);
		}
	};
	template<typename T> constexpr T little2big_f(T t){
		return convert<little_endian_tag, big_endian_tag>(t);
	}
	template<typename T> constexpr T big2little_f(T t){
		return convert<big_endian_tag, little_endian_tag>(t);
	}

	// little <--> local
	template<typename T> struct little2local{
		static constexpr T operator()(T t) {
			return endian::convert<little_endian_tag, local_endian_tag>(t);
		}
	};
	template<typename T> struct local2little{
		static constexpr T operator()(T t) {
			return endian::convert<local_endian_tag, little_endian_tag>(t);
		}
	};
	template<typename T> constexpr T local2little_f( T t){
		return endian::convert<local_endian_tag, little_endian_tag>(t);
	}
	template<typename T> constexpr T little2local_f( T t){
		return endian::convert<little_endian_tag, local_endian_tag>(t);
	}

	// big <--> local
	template<typename T> struct big2local{
		static constexpr T operator()(T t) {
			return endian::convert<big_endian_tag, local_endian_tag>(t);
		}
	};
	template<typename T> struct local2big{
		static constexpr T operator()(T t) {
			return endian::convert<local_endian_tag, big_endian_tag>(t);
		}
	};
	template<typename T> constexpr T local2big_f( T t){
		return endian::convert<local_endian_tag, big_endian_tag>(t);
	}
	template<typename T> constexpr T big2local_f( T t){
		return endian::convert<big_endian_tag, local_endian_tag>(t);
	}
}

#endif //end NIX_COMMON_ENDIAN_H__
