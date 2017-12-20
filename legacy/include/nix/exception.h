//XIRANG2_LICENSE_PLACE_HOLDER

#ifndef XR_EXCEPTION_H__
#define XR_EXCEPTION_H__
#include <xirang2/config.h>
#include <xirang2/macro_helper.h>
#include <xirang2/utf8char.h>

#ifdef XR_DERIVED_FROM_STD_EXCEPTION
#include <exception>
#endif


#include <xirang2/config/abi_prefix.h>
namespace xirang2
{
	struct XR_API exception
#ifdef XR_DERIVED_FROM_STD_EXCEPTION
		:public std::exception
#endif
	{
		virtual const char_utf8* what() const XR_COMPATIBLE_NOTHROW()  = 0;
		virtual exception& append(const char_utf8* info)  = 0;
		virtual ~exception() XR_COMPATIBLE_NOTHROW() {};
	};
}

#define XR_EXCEPTION_TYPE_EX(type, base) \
	struct XR_API type : public base {\
	virtual ~type() XR_COMPATIBLE_NOTHROW() {}\
	}

#define XR_EXCEPTION_TYPE(type) \
	XR_EXCEPTION_TYPE_EX(type, ::xirang2::exception)

#define XR_INTERNAL_EXCEPTION_TYPE_EX(type, base) \
	struct type : public base {\
	virtual ~type() XR_COMPATIBLE_NOTHROW() {}\
	}

#define XR_INTERNAL_EXCEPTION_TYPE(type) \
	XR_INTERNAL_EXCEPTION_TYPE_EX(type, ::xirang2::exception)

#include <xirang2/config/abi_suffix.h>
#endif //end XR_EXCEPTION_H__
