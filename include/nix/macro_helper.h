#ifndef NIX_MACRO_HELPER_H__
#define NIX_MACRO_HELPER_H__

#define NIX_JOIN( X, Y ) NIX_DO_JOIN( X, Y )
#define NIX_DO_JOIN( X, Y ) NIX_DO_JOIN2(X,Y)
#define NIX_DO_JOIN2( X, Y ) X##Y

#ifndef NIX_STRING
#  define NIX_STRING_(x) #x
#  define NIX_STRING(x) NIX_STRING_(x) 
#endif

#ifndef NIX_WIDE_STRING
#  define NIX_WIDE_STRING_(str) L ## str
#  define NIX_WIDE_STRING(str) NIX_WIDE_STRING_(str)
#endif

#endif //NIX_MACRO_HELPER_H__

