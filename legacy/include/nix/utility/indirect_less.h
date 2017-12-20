#ifndef XIRANG2_UTILITY_INDIRECT_LESS_H__
#define XIRANG2_UTILITY_INDIRECT_LESS_H__
#include <xirang2/config.h>
namespace xirang2{
	struct indirect_less
	{
		template<typename T>
		bool operator()(const T& lhs, const T& rhs)const
		{
			return *lhs < *rhs;
		}
	};
}

#endif //end XIRANG2_UTILITY_INDIRECT_LESS_H__



