#ifndef XR_COMMON_BYTE_ORDER_H__
#define XR_COMMON_BYTE_ORDER_H__

#include <xirang2/endian.h>

namespace xirang2{

	namespace byteorder{

		template<typename From, typename To, typename T, std::size_t N> 
			struct convert_imp;

		template<typename T, std::size_t N> 
			struct convert_imp<little_endian_tag, big_endian_tag, T, N>{ 
				static T apply(T t){
					char* p = (char*)&t;
					std::reverse(p, p + sizeof(T));
					return t;
				}
			};
		template<typename T, std::size_t N> 
			struct convert_imp<big_endian_tag, little_endian_tag, T, N>{ 
				static T apply(T t){
					return convert_imp<little_endian_tag, big_endian_tag, T, N>::apply(t);
				}
			};

		template<typename From, typename T, std::size_t N> 
			struct convert_imp<From, From, T, N>{
				static T apply(T t){
					return t;
				}
			};

		template<typename From, typename To, typename T> 
			T convert(T t){
				return convert_imp<From, To, T, sizeof(T)>::apply(t);
			}

		template<typename T> T little2big(T t){
			return convert<little_endian_tag, big_endian_tag>(t);
		}
		template<typename T> T bit2little(T t){
			return convert<big_endian_tag, little_endian_tag>(t);
		}

		//TODO:
		template<typename T> T little2pdp(T t);
		template<typename T> T pdp2little(T t);
		template<typename T> T pdp2big(T t);
		template<typename T> T big2pdp(T t);

	}

	template<typename T> struct local2ex{
		T operator()(T t) const {
			return byteorder::convert<local_endian_tag, exchange_endian_tag>(t);
		}
	};
	template<typename T> struct ex2local{
		T operator()(T t) const {
			return byteorder::convert<exchange_endian_tag, local_endian_tag>(t);
		}
	};
	template<typename T> T local2ex_f( T t){
		return byteorder::convert<local_endian_tag, exchange_endian_tag>(t);
	}
	template<typename T> T ex2local_f( T t){
		return byteorder::convert<exchange_endian_tag, local_endian_tag>(t);
	}

};

#endif //end XR_COMMON_BYTE_ORDER_H__
