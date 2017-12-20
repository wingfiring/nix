#ifndef XR_XIRANG2_SERIALIZE_H__
#define XR_XIRANG2_SERIALIZE_H__

#include <xirang2/io.h>
#include <xirang2/serialize/s11n.h>
#include <xirang2/type/object.h>
#include <xirang2/type/binder.h>

//TODO: recheck
namespace xirang2{ namespace type{ namespace io{ namespace exchange{

	// default imp of type method
	template<typename T> struct serializer{
		static xirang2::io::writer& apply(xirang2::io::writer& wt, ConstCommonObject obj){
			return xirang2::io::exchange::as_sink(wt) & uncheckBind<T>(obj);
		}
	};

	template<typename T> struct deserializer{
		static xirang2::io::reader& apply(xirang2::io::reader& rd, CommonObject obj, heap& inner, ext_heap& ext){
			return xirang2::io::exchange::as_source(rd) & uncheckBind<T>(obj);
		}
	};
}}}}

#endif //end XR_XIRANG2_SERIALIZE_H__

