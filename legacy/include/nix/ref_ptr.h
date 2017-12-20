//XIRANG2_LICENSE_PLACE_HOLDER

#ifndef XR_REFERENCE_PTR_H__
#define XR_REFERENCE_PTR_H__
#include <xirang2/config.h>

#include <xirang2/config/abi_prefix.h>
namespace xirang2
{
///handler ptr, it's a value semantic
    template<typename T>
    class ref_ptr
    {
    public:

		///ctor
		template<typename U>
        explicit ref_ptr(U &p) : m_ptr(&p){}

		///ctor
		template<typename U>
        ref_ptr(const ref_ptr<U>& p) : m_ptr(&p.get()){}

		///get hold pointer
        T& get() const {
            return *m_ptr;
        }
        
		///member access
		///@pre !is_nullptr(*this)
        T* get_pointer() const{
            return m_ptr;
        }
        
		 
		///deference
		///@pre !is_nullptr(*this)
        operator T&() const{
            return *m_ptr;
        }

		///swap
		ref_ptr& swap(ref_ptr& rhs)
        {
            std::swap(m_ptr, rhs.m_ptr);
            return *this;
        }

    private:        
        T* m_ptr;
    };

	//swap
    template<typename T>
    void swap(ref_ptr<T>& lhs, ref_ptr<T>& rhs)
    {
        lhs.swap(rhs);
    }		

	template<class T> inline ref_ptr<T> ref(T & t)
	{ 
		return ref_ptr<T>(t);
	}

	template<class T> inline ref_ptr<T const> cref(T const & t)
	{
		return ref_ptr<T const>(t);
	}
}
#include <xirang2/config/abi_suffix.h>
#endif //XR_REFERENCE_PTR_H__
