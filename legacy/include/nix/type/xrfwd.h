#ifndef XIRANG2_XRFWD_H__
#define XIRANG2_XRFWD_H__

#include <xirang2/type/xrbase.h>
#include <xirang2/path.h>
#include <iterator>
#include <xirang2/backward/unique_ptr.h>

namespace xirang2{ namespace vfs{
	class IVfs;
	class RootFs;
}}

namespace xirang2 { namespace type{
  class TypeMethods;
  XR_API extern TypeMethods& DefaultMethods();

  struct Serializer;
  struct DeSerializer;


  //handle of a type member item owned by Type.
  class TypeItem;
  typedef BiRangeT<const_itr_traits<TypeItem> > TypeItemRange;

  class TypeArg;
  typedef BiRangeT<const_itr_traits<TypeArg> > TypeArgRange;

  //handle of a type owned by Xirang
  class Type;
  typedef BiRangeT<const_itr_traits<Type> >  TypeRange;

  class TypeSynonym;

  typedef BiRangeT<const_itr_traits<TypeSynonym>> TypeSynonymRange;

  class TypeAlias;
  typedef BiRangeT<const_itr_traits<TypeAlias> > TypeAliasRange;

  class Namespace;
  typedef BiRangeT<const_itr_traits<Namespace> > NamespaceRange;

  //handle of a subobject of object woned by Xirang.
  class SubObject;
  typedef BiRangeT<const_itr_traits<SubObject> > SubObjRange;

  //handle of an object owned by Xirang
  class CommonObject;

  struct ConstNameValuePair;
  struct NameValuePair;

  typedef BiRangeT<const_itr_traits<NameValuePair> > ObjectRange;

  class ConstSubObject;
  typedef BiRangeT<const_itr_traits<ConstSubObject> > ConstSubObjRange;

  class ConstCommonObject;
  typedef BiRangeT<const_itr_traits<ConstCommonObject> > ConstObjectRange;

  class Xirang;			//runtime

  typedef int ErrorCode;

  class TypeBuilder;
  class NamespaceBuilder;
  class TypeAliasBuilder;

  template<typename Imp>
  class ImpAccessor;

  inline int comparePtr(const void* p1, const void* p2) {
	  if (p1 < p2)
          return -1;
	  if (p1 > p2)
          return 1;
      return 0;
  }

  template<typename T> struct assigner;
	template<typename T> struct constructor;
	template<typename T> struct destructor;
	template<typename T> struct comparison;
	template<typename T> struct hasher;
	template<typename T> struct layout;
	template<typename T> struct extendMethods;

	template<typename T> constructor<T> get_constructor(T*);
	template<typename T> destructor<T> get_destructor(T*);
	template<typename T> assigner<T> get_assigner(T*);
	template<typename T> layout<T> get_layout(T*);
	template<typename T> extendMethods<T> get_extendMethods(T*);

	template<typename T>
		int compare(const T& lhs, const T& rhs){ return lhs.compare(rhs);}
}}


#ifndef DEFINE_COMPARE
#define DEFINE_COMPARE(T)\
inline bool operator==(const T& lhs, const T& rhs)\
{ return xirang2::type::compare(lhs, rhs) == 0;}\
inline bool operator!=(const T& lhs, const T& rhs)\
{ return xirang2::type::compare(lhs, rhs) != 0;}\
inline bool operator<(const T& lhs, const T& rhs)\
{ return xirang2::type::compare(lhs, rhs) < 0;}\
inline bool operator<=(const T& lhs, const T& rhs)\
{ return xirang2::type::compare(lhs, rhs) <= 0;}\
inline bool operator>(const T& lhs, const T& rhs)\
{ return xirang2::type::compare(lhs, rhs) > 0;}\
inline bool operator>=(const T& lhs, const T& rhs)\
{ return xirang2::type::compare(lhs, rhs) >= 0;}
#endif

#endif				//end XIRANG2_XRFWD_H__
