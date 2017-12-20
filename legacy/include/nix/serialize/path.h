#ifndef XIRANG2_IO_PATH_H__
#define XIRANG2_IO_PATH_H__
#include <xirang2/serialize/s11n.h>
#include <xirang2/path.h>
namespace xirang2{
	template<typename Ar, typename =
		typename std::enable_if<io::s11n::is_deserializer<Ar>::value>::type>
	Ar& load (Ar& ar, file_path& path)
	{
		string p = load<string>(ar);
		path = file_path(p, pp_none);
		return ar;
	}
	template<typename Ar, typename =
		typename std::enable_if< io::s11n::is_serializer<Ar>::value>::type>
	Ar& save(Ar& ar, const file_path& path)
	{
		return ar & path.str();
	}

	template<typename Ar, typename =
		typename std::enable_if<io::s11n::is_deserializer<Ar>::value>::type>
	Ar& load (Ar& ar, simple_path& path)
	{
		string p = load<string>(ar);
		path = simple_path(p, pp_none);
		return ar;
	}
	template<typename Ar, typename =
		typename std::enable_if< io::s11n::is_serializer<Ar>::value>::type>
	Ar& save(Ar& ar, const simple_path& path)
	{
		return ar & path.str();
	}
}
#endif //end XIRANG2_IO_PATH_H__

