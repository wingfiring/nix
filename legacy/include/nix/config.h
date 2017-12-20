//XIRANG2_LICENSE_PLACE_HOLDER

#ifndef XR_COMMON_CONFIG_H__
#define XR_COMMON_CONFIG_H__

#include <xirang2/config/config.h>
#include <xirang2/config/user.h>

#if defined (XR_COMMON_DLL_EXPORT)
#	define XR_API XR_DLL_EXPORT
#	define XR_INTERFACE XR_DLL_INTERFACE
#elif defined (XR_COMMON_DLL_IMPORT)
#	define XR_API XR_DLL_IMPORT
#	define XR_INTERFACE XR_DLL_INTERFACE
#else
#	define XR_API			//for static
#	define XR_INTERFACE 
#endif

template<typename... T>
inline void unuse(const T& ...) {}
struct no_initialize{};
namespace xirang2{

#ifndef MSVC_COMPILER_
	template<typename T> constexpr T const& const_max(T const& a, T const& b) {
		return a < b ? b : a;
	}
	template<typename T> constexpr T const& const_min(T const& a, T const& b) {
		return a < b ? a : b;
	}
#endif
	struct null_type;
	struct empty_type{};

	template<typename T>
		void check_delete(T* p){
			static_assert(sizeof(T) > 0, "Must not delete a incomplete type");
			delete p;
		}

}

#endif //end XR_COMMON_CONFIG_H__


