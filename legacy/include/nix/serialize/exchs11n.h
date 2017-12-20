#ifndef XR_COMMON_IO_S11N_EXCHANGE_H__
#define XR_COMMON_IO_S11N_EXCHANGE_H__
#include <xirang2/serialize/s11nbase.h>
#include <xirang2/byteorder.h>
#include <xirang2/type/nativetype.h>
#include <xirang2/utility.h>
#include <limits>

namespace xirang2{ namespace io{ namespace exchange{

	template<typename Ar> struct serializer : public s11n::serializer_base<Ar>, public io::writer{
		explicit serializer(Ar& ar) : s11n::serializer_base<Ar>(ar){};

		range<const byte*> write(const range<const byte*>& r){
			return this->get().write(r);
		}

		bool writable() const{
			return this->get().writable();
		}

		/// sync buffer, implementationi defines.
		void sync(){
			return this->get().sync();
		}
	};
	template<typename Ar> struct deserializer : public s11n::deserializer_base<Ar> , public io::reader{
		explicit deserializer(Ar& ar) : s11n::deserializer_base<Ar>(ar){};
		range<byte*> read(const range<byte*>& buf){
			return this->get().read(buf);
		}

		bool readable() const {
			return this->get().readable();
		}
	};
	template<typename Ar> serializer<Ar> as_sink(Ar& ar){ return serializer<Ar>(ar);}
	template<typename Ar> deserializer<Ar> as_source(Ar& ar){ return deserializer<Ar>(ar);}

	//TODO: map T to a exchangable type U
	template<typename T> struct exchange_type_of{ typedef T type;};

	template<> struct exchange_type_of<wchar_t>{ typedef xirang2::int32_t type;};
	template<> struct exchange_type_of<char>{ typedef xirang2::int8_t type;};
	template<> struct exchange_type_of<bool>{ typedef xirang2::int8_t type;};

	XR_EXCEPTION_TYPE(bad_exchange_cast);

	template<typename T, typename U> struct exchange_cast_imp{
		static U apply(T t){
			if ((t < std::numeric_limits<U>::min())
					|| (t > std::numeric_limits<U>::max()))
				XR_THROW(bad_exchange_cast);
			return U(t);
		}
	};
	template<typename U> struct exchange_cast_imp<bool, U>{
		static U apply(bool t){
			return U(t ? 1 : 0);
		}
	};
	template<typename T> struct exchange_cast_imp<T, bool>{
		static bool apply(T t){
			return t != 0;
		}
	};

	template<typename T> struct exchange_cast_imp<T, T>{
		static T apply(T t){
			return t;
		}
	};

	template<typename U, typename T> U exchange_cast(T t){
		return exchange_cast_imp<T, U>::apply(t);
	}
	template<typename U, typename T> struct exchange_caster{
		U operator()(T t) const{
			return exchange_cast<U>(t);
		}
	};

	template<typename Ar, typename T,
		typename = typename std::enable_if<std::is_scalar<T>::value &&
			s11n::is_serializer<Ar>::value>::type>
		Ar& save(Ar& wt, const T& v)
		{
			typedef typename exchange_type_of<T>::type U;
			U t = exchange_cast<U>(v);

			const byte* first = reinterpret_cast<const byte*>(&t);
			const byte* last = reinterpret_cast<const byte*>(&t + 1);

			io::block_write(get_interface<io::writer>(wt.get()), make_range(first, last));

			return wt;
		}

	template<typename Ar, typename T,
		typename = typename std::enable_if<std::is_scalar<T>::value &&
			s11n::is_deserializer<Ar>::value>::type>
		Ar& load(Ar& rd, T& v)
		{
			typedef typename exchange_type_of<T>::type exch_type;
			exch_type t;
			byte* first = reinterpret_cast<byte*>(&t);
			byte* last = reinterpret_cast<byte*>(&t + 1);

			if (!io::block_read(get_interface<io::reader>(rd.get()), make_range(first, last)).empty() )
				XR_THROW(io::read_exception);

			v = exchange_cast<T>(ex2local_f(t));

			return rd;
		}

	template<typename Ar, typename T,
		typename = typename std::enable_if<std::is_empty<T>::value &&
			s11n::is_serializer<Ar>::value>::type>
		Ar& save(Ar& wt, const T&, dummy<0> =0)
		{
			return wt;
		}

	template<typename Ar, typename T,
		typename = typename std::enable_if<std::is_empty<T>::value &&
			s11n::is_deserializer<Ar>::value>::type>
		Ar& load(Ar& rd, T&, dummy<0> =0)
		{
			return rd;
		}
}}}

#endif //end XR_COMMON_IO_S11N_EXCHANGE_H__

