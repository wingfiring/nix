#ifndef XIRANG2_SHA1_PROCESS_VALUE_H__
#define XIRANG2_SHA1_PROCESS_VALUE_H__

#include <xirang2/sha1.h>
#include <xirang2/serialize/exchs11n.h>

#include <xirang2/buffer.h>
#include <xirang2/string.h>
#include <xirang2/versiontype.h>

namespace xirang2{
	template<typename T, typename = typename std::enable_if<std::is_scalar<T>::value, void>::type>
	inline void process_value(sha1& sha, T t){
		byte const * p = (byte const*)&t;
		sha.write(make_range(p, p + sizeof(t)));
	}
}
#endif //end XIRANG2_SHA1_PROCESS_VALUE_H__
