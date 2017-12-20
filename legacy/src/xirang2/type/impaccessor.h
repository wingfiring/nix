#ifndef XIRANG2_SRC_XIRANG2_IMP_ACCESSOR_H
#define XIRANG2_SRC_XIRANG2_IMP_ACCESSOR_H
#include <xirang2/type/xrfwd.h>

namespace xirang2{ namespace type{
  template<typename Imp>
  class ImpAccessor
  {
  public:
      template<typename T>
      static Imp* const & getImp(const T& t) { return t.m_imp;}
  };

}}
#endif
