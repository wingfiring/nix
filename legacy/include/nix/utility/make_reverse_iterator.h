#ifndef XIRANG2_UTILITY_MAKE_REVERSE_ITERATOR_H__
#define XIRANG2_UTILITY_MAKE_REVERSE_ITERATOR_H__

#include <xirang2/config.h>
#include <iterator>
namespace xirang2{
	template<typename Itr> std::reverse_iterator<Itr> make_reverse_iterator(Itr itr){
		return std::reverse_iterator<Itr>(std::move(itr));
	}
}


#endif //end XIRANG2_UTILITY_MAKE_REVERSE_ITERATOR_H__

