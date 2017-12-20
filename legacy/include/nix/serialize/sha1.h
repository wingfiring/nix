#ifndef XIRANG2_IO_SHA1_DIGEST_H__
#define XIRANG2_IO_SHA1_DIGEST_H__
#include <xirang2/serialize/s11n.h>
#include <xirang2/sha1.h>
namespace xirang2{
	template<typename Ar, typename =
		typename std::enable_if<io::s11n::is_deserializer<Ar>::value>::type>
	Ar& load (Ar& ar, sha1_digest& dig)
	{
		for (auto& i : dig.v)
			ar & i;
		return ar;
	}
	template<typename Ar, typename =
		typename std::enable_if< io::s11n::is_serializer<Ar>::value>::type>
	Ar& save(Ar& ar, const sha1_digest& dig)
	{
		for (auto i : dig.v)
			ar & i;
		return ar;
	}


}
#endif //XIRANG2_IO_SHA1_DIGEST_H__
