#ifndef XR_COMMON_ENDIAN_H__
#define XR_COMMON_ENDIAN_H__

#include <xirang2/config.h>
#ifdef LINUX_OS_
#include <endian.h>
#elif defined (MACOS_OS_)
#include <machine/endian.h>
#elif defined (WIN32_OS_)
#ifndef __LITTLE_ENDIAN
#define __LITTLE_ENDIAN 1234
#define __BIG_ENDIAN    4321
#endif // __LITTLE_ENDIAN
#ifndef __BYTE_ORDER
#define __BYTE_ORDER __LITTLE_ENDIAN
#endif  // __BYTE_ORDER
#else
#error "Not support"
#endif
namespace xirang2{
	struct little_endian_tag{};
	struct big_endian_tag{};
	struct pdp_endian_tag{};
	typedef little_endian_tag exchange_endian_tag;

# if (__BYTE_ORDER == __LITTLE_ENDIAN)
#  define XR_LITTLE_ENDIAN
	typedef little_endian_tag local_endian_tag;
# elif (__BYTE_ORDER == __BIG_ENDIAN)
#  define XR_BIG_ENDIAN
	typedef big_endian_tag local_endian_tag;
# elif (__BYTE_ORDER == __PDP_ENDIAN)
#  define XR_PDP_ENDIAN
	typedef pdp_endian_tag local_endian_tag;
# else
#  error Unknown machine endianness detected.
# endif
# define XR_BYTE_ORDER __BYTE_ORDER

}


#endif //end XR_COMMON_ENDIAN_H__
