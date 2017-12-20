#ifndef XIRANG2_XRBASE_H__
#define XIRANG2_XRBASE_H__

#include <xirang2/range.h>
#include <xirang2/string.h>
#include <xirang2/iterator.h>
#include <xirang2/buffer.h>
#include <xirang2/context_except.h>

namespace xirang2{
    typedef ext_heap::handle handle;
    
    template<typename T>
    struct BiRangeT : public  range<bidir_iterator<T> >
    {
        typedef range<bidir_iterator<T> > base;
        typedef typename base::iterator iterator;
        BiRangeT() {};
        BiRangeT(const iterator& first, const iterator& last) : base(first, last){}
        
    };
    
    template<typename T>
    struct ForwardRangeT : public  range<forward_iterator<T> >
    {
        typedef range<forward_iterator<T> > base;
        typedef typename base::iterator iterator;
        ForwardRangeT() {};
        ForwardRangeT(const iterator& first, const iterator& last) : base(first, last){}
        
    };
}

#endif //end XIRANG2_XRBASE_H__

