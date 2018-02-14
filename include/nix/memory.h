//NIX_LICENSE_PLACE_HOLDER

#ifndef NIX_MEMORY_H__
#define NIX_MEMORY_H__
#include <nix/config.h>
#include <nix/contract.h>
#include <nix/memory/heap.h>

//STD
#include <memory>

namespace nix
{
	/// return the max necessary alignment.
	NIX_API extern std::size_t max_alignment();

	/// allocate memory for given type 
	/// \param h given heap
	/// \return not null or throw
	template<typename T> inline T* malloc_obj(heap& h){
		return h.malloc(sizeof(T), alignof(T), 0);
	}
	/// deallocate memory pointed by arg p
	template<typename T> inline void free_obj(heap& h, T* p) noexcept{
		h.free(p, sizeof(T), alignof(T));
	}

	/// this class intends to be used as a functor bounded free_obj with a heap 
	template<typename T> class deallocator
	{
		heap* m_heap;
	public:
		/// \ctor
		constexpr explicit deallocator(heap& h) noexcept :m_heap(&h){}
		/// forward the kept heap and parameter p to free_obj
		void operator()(T* p) const noexcept{
			free_obj(*m_heap, p);
		}
	};

	/// heap deletor
	template<typename T> class heap_deletor
	{
		heap* m_heap;
	public:
		/// \ctor
		constexpr explicit heap_deletor(heap& h) noexcept :m_heap(&h){}
		/// destruct given object and deallocate memory
		void operator()(T* p) const noexcept{
			if (p) {
				p->~T();
				free_obj(*m_heap, p);
			}
		}

	};

	/// intends to hold the memory before object constructed
	/// once the object is constructed, it should be released immediately to avoid deallocated unexpected. 
	/// the released pointer could be hold by smart pointer such as std::unique_ptr/shared_ptr with customized deleter, such as heap_deletor
	template<typename T> class uninitialized_heap_ptr
	{
		heap* m_heap;
		T* m_p;
	public:
		/// \ctor hold the given heap, and allocate memory for object of type T
		explicit uninitialized_heap_ptr(heap& h)
			: m_heap(&h)
			, m_p(malloc_obj<T>(h))
		{}
		/// \ctor accept an allocated but not constructed object pointer and hold it.
		explicit uninitialized_heap_ptr(T* p, heap& h) noexcept : m_p(p), m_heap(&h){}

		/// if the hold pointer is not released, free it
		~uninitialized_heap_ptr() noexcept{
			if (m_p) free_obj(*m_heap, m_p);
		}

		/// release the ownership 
		/// \post get() == nullptr
		/// \note this method should be called immediately once the object is constructed.
		T* release() noexcept
		{
			return std::exchange(m_p, nullptr);
		}

		/// retrieve the hold object pointer
		T* get() const noexcept { return m_p;}
	};

	/// make an std::unique_ptr and use heap_deletor as deletor
	template<typename T, typename ... Args> std::unique_ptr<T, heap_deletor<T>> make_unique_with_heap(heap&h, Args&& ... args){
		uninitialized_heap_ptr<T> p (h);
		new (p.get()) T(std::forward<Args>(args)...);
		return std::unique_ptr<T, heap_deletor<T>>(p.release(), heap_deletor<T>(h));
	}
	/// make an std::unique_ptr and use heap_deletor as deletor
	template<typename T, typename ... Args> std::shared_ptr<T> make_shared_with_heap(heap&h, Args&& ... args){
		uninitialized_heap_ptr<T> p (h);
		new (p.get()) T(std::forward<Args>(args)...);
		return std::shared_ptr<T>(p.release(), heap_deletor<T>(h));
	}
}

#endif //end NIX_STRING_H__

