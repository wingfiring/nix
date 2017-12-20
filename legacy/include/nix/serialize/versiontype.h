#ifndef XIRANG2_IO_VERSION_TYPE_H__
#define XIRANG2_IO_VERSION_TYPE_H__
#include <xirang2/serialize/s11n.h>
#include <xirang2/serialize/sha1.h>
#include <xirang2/versiontype.h>
namespace xirang2{
	template<typename Ar, typename =
		typename std::enable_if<io::s11n::is_deserializer<Ar>::value>::type>
	Ar& load(Ar& ar, version_type& ver)
	{
		return ar & ver.protocol_version
			& ver.algorithm
			& ver.id
			& ver.conflict_id;
	}

	template<typename Ar, typename =
		typename std::enable_if< io::s11n::is_serializer<Ar>::value>::type>
	Ar& save(Ar& ar, const version_type& ver)
	{
		ar & ver.protocol_version
			& ver.algorithm
			& ver.id
			& ver.conflict_id;
		return ar;
	}


}
#endif //end XIRANG2_IO_VERSION_TYPE_H__

