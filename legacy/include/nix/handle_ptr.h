//XIRANG2_LICENSE_PLACE_HOLDER

#ifndef XR_HANDLE_PTR_H__
#define XR_HANDLE_PTR_H__
#include <xirang2/config.h>
#include <xirang2/assert.h>
#include <xirang2/operators.h>

#include <xirang2/config/abi_prefix.h>
namespace xirang2
{
	///handler ptr, it's a value semantic
    template<typename T>
    class handle_ptr 
    {
    public:
		typedef T element_type;	///< is a smart pointer

		handle_ptr() : m_ptr(0){}

		///ctor
		template<typename U>
        explicit handle_ptr(U *p) : m_ptr(p){}

		///ctor
		template<typename U>
        handle_ptr(const handle_ptr<U>& p) : m_ptr(p.get()){}

		///get hold pointer
        T* get() const {
            return m_ptr;
        }
        
		///member access
		///@pre !is_null(*this)
        T* operator->() const{
            XR_PRE_CONDITION(m_ptr != 0 && "dereference null pointer");
            return m_ptr;
        }
        
		 
		///deference
		///@pre !is_null(*this)
        T& operator*() const{
            XR_PRE_CONDITION(m_ptr != 0 && "dereference null pointer");
            return *m_ptr;
        }

		///swap
		handle_ptr& swap(handle_ptr& rhs)
        {
            std::swap(m_ptr, rhs.m_ptr);
            return *this;
        }

		 
		///@post is_null(*this)
        T* release() {
            T* ret = m_ptr;
            m_ptr = 0;
            return ret;
        }


		///reset
        void reset(T *p = 0)
        {
            m_ptr = p;
        }

		///
		bool is_null() const { return get() == 0;}

		explicit operator bool() const { return get() != 0;}
    private:        
        T* m_ptr;
    };

	template<typename T, typename U>
	bool operator < (const handle_ptr<T>& lhs, const handle_ptr<U>& rhs) { return lhs.get() < rhs.get();}

	template<typename T, typename U>
	bool operator == (const handle_ptr<T>& lhs, const handle_ptr<U>& rhs) { return lhs.get() == rhs.get();}

	template<typename T, typename U>
		class handle_ptr_operator : totally_ordered<handle_ptr<T>, handle_ptr<U>>
	{};

	//swap
    template<typename T>
    void swap(handle_ptr<T>& lhs, handle_ptr<T>& rhs)
    {
        lhs.swap(rhs);
    }
}
#include <xirang2/config/abi_suffix.h>
#endif //XR_HANDLE_PTR_H__
