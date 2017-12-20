#ifndef XIRANG2_UTILITY_INIT_ONCE_H__
#define XIRANG2_UTILITY_INIT_ONCE_H__
#include <xirang2/config.h>
namespace xirang2{
	template<typename T>
	class init_once
	{
		static int refcount;
	public:
		init_once()
		{
			if (refcount++ == 0)
			  	T::init();
		}
		~init_once()
		{
			if (refcount == 0)
				T::uninit();
		}
	};

	template<typename T>
	int init_once<T>::refcount = 0;

#define XR_INIT_ONCE(T)\
	namespace { \
	::xirang2::init_once<T> g_init_once;}


}
#endif //end XIRANG2_UTILITY_INIT_ONCE_H__


