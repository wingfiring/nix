#ifndef XIRANG2_UTILITY_UNUSED_H__
#define XIRANG2_UTILITY_UNUSED_H__

#include <xirang2/config.h>
namespace xirang2{
	//use to surppress unused variable warning.
	template<typename ...T>
	void unused(const T&... ) {};
}


#endif //end XIRANG2_UTILITY_UNUSED_H__

