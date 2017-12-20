#ifndef XIRANG2_TYPE_STRING_H
#define XIRANG2_TYPE_STRING_H
#include <xirang2/config.h>
#include <xirang2/string.h>
#include <xirang2/string_algo/string.h>

namespace xirang2{
#if defined(CLANG_COMPILER_) || defined(GNUC_COMPILER_)
	template<typename T> const_range_string type_string() {
		const_range_string s(__PRETTY_FUNCTION__);
		auto pos = rfind(s, '=');
		return const_range_string(pos + 2, s.end() - 1 );
	}
#elif defined(MSVC_COMPILER_)
	template<typename T> const_range_string type_string() {
		const_range_string s(__FUNCSIG__);
		auto pos = find(s, '<');
        pos = find(pos + 1, s.end(), '<');
		return const_range_string(pos + 1, s.end() - 7 );

	}
#else
#error "Unknow compiler"
#endif
}

#endif // end XIRANG2_TYPE_STRING_H 

