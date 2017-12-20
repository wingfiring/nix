//XIRANG2_LICENSE_PLACE_HOLDER

#ifndef XR_MACRO_HELPER_H__
#define XR_MACRO_HELPER_H__

#define XR_JOIN( X, Y ) XR_DO_JOIN( X, Y )
#define XR_DO_JOIN( X, Y ) XR_DO_JOIN2(X,Y)
#define XR_DO_JOIN2( X, Y ) X##Y

#ifndef XR_STRING
#  define XR_STRING_(x) #x
#  define XR_STRING(x) XR_STRING_(x) 
#endif

#ifndef XR_WIDE_STRING
#  define XR_WIDE_STRING_(str) L ## str
#  define XR_WIDE_STRING(str) XR_WIDE_STRING_(str)
#endif

#define XR_MP(x) XR_WIDE_STRING(XR_STRING(x)), x

#ifndef XR_EXCEPTION_SPEC_ENABLED

#  define XR_COMPATIBLE_NOTHROW() throw()
#  define XR_NOTHROW() 
#  define XR_THROW0() 
#  define XR_THROW1(e1)
#  define XR_THROW2(e1, e2)
#  define XR_THROW3(e1, e2, e3)
#  define XR_THROW4(e1, e2, e3, e4)
#  define XR_THROW5(e1, e2, e3, e4, e5)
#  define XR_THROW6(e1, e2, e3, e4, e5, e6)
#  define XR_THROW7(e1, e2, e3, e4, e5, e6, e7)
#  define XR_THROW8(e1, e2, e3, e4, e5, e6, e7, e8)
#  define XR_THROW9(e1, e2, e3, e4, e5, e6, e7, e8, e9)

#else

#  define XR_COMPATIBLE_NOTHROW() throw()
#  define XR_NOTHROW() throw()
#  define XR_THROW0() throw()
#  define XR_THROW1(e1) throw (e1)
#  define XR_THROW2(e1, e2) throw (e1, e2)
#  define XR_THROW3(e1, e2, e3) throw (e1, e2, e3)
#  define XR_THROW4(e1, e2, e3, e4) throw (e1, e2, e3, e4)
#  define XR_THROW5(e1, e2, e3, e4, e5) throw (e1, e2, e3, e4, e5)
#  define XR_THROW6(e1, e2, e3, e4, e5, e6) throw (e1, e2, e3, e4, e5, e6)
#  define XR_THROW7(e1, e2, e3, e4, e5, e6, e7) throw (e1, e2, e3, e4, e5, e6, e7)
#  define XR_THROW8(e1, e2, e3, e4, e5, e6, e7, e8) throw (e1, e2, e3, e4, e5, e6, e7, e8)
#  define XR_THROW9(e1, e2, e3, e4, e5, e6, e7, e8, e9) throw (e1, e2, e3, e4, e5, e6, e7, e8, e9)

#endif //end XR_EXCEPTION_SPEC_ENABLED

#define DISABLE_CLONE(type)\
	private:\
	type(const type&);\
	type& operator=(const type&)

#endif //XR_MACRO_HELPER_H__

