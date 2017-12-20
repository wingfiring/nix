#define XR_ABI_PREFIX <xirang2/config/abi/msvc_prefix.h>
#define XR_ABI_SUFFIX <xirang2/config/abi/msvc_suffix.h>

#define XR_FUNCTION __FUNCTION__ 

#define XR_DLL_EXPORT __declspec( dllexport )
#define XR_DLL_IMPORT __declspec( dllimport )

#define XR_DLL_INTERFACE __declspec(novtable)

//for c++0x explicit operator
#define EXPLICIT_OPERATOR 

