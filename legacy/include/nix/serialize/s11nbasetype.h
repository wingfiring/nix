#ifndef XIRANG2_IO_EXCH_BASE_TYPE_H__
#define XIRANG2_IO_EXCH_BASE_TYPE_H__
#include <xirang2/serialize/exchs11n.h>
namespace xirang2{ namespace io{namespace exchange{
	template<typename Ar> Ar& save_size_t(Ar& ar, std::size_t s){
		return save(ar, exchange_cast<uint32_t>(s));
	}
	template<typename Ar> Ar& load_size_t(Ar& ar, std::size_t& s){
		s = exchange_cast<std::size_t>(load<uint32_t>(ar));
		return ar;
	}
	template<typename Ar> std::size_t load_size_t(Ar& ar){
		return exchange_cast<std::size_t>(load<uint32_t>(ar));
	}
}
namespace local{
	template<typename Ar> Ar& save_size_t(Ar& ar, std::size_t s){
		return save(ar, s);
	}
	template<typename Ar> Ar& load_size_t(Ar& ar, std::size_t& s){
		return load(ar, s);
	}
	template<typename Ar> std::size_t load_size_t(Ar& ar){
		return load<std::size_t>(ar);
	}

}}}
#endif //end XIRANG2_IO_EXCH_BASE_TYPE_H__

