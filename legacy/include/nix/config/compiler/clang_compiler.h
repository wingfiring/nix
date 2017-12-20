#define XR_FUNCTION __PRETTY_FUNCTION__ 

#define XR_DLL_EXPORT __attribute__ ((visibility ("default")))
#define XR_DLL_IMPORT __attribute__ ((visibility ("default")))

#ifdef EXPLICIT_OPERATOR
#undef EXPLICIT_OPERATOR
#endif
#define EXPLICIT_OPERATOR 

#define XR_DLL_INTERFACE 
