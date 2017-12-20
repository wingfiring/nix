#ifndef XR_IMP_SHARED_DATA_H__
#define XR_IMP_SHARED_DATA_H__

#include <xirang2/config.h>

#include <atomic>
#include <xirang2/config/abi_prefix.h>
namespace xirang2{ 
	struct heap;
	namespace private_{

	template<typename T> struct shared_data
	{
		heap* heap_ptr;
        // C++ standard requires:
        // "These specializations have standard layout, trivial default constructors, and trivial destructors. 
        // They support aggregate initialization syntax."
        // It means that imp can't change the member layout, so we can use std atomic here and expose it.
        // In VC120, the base _Atomic_impl<> has a data member _My_flag,  but all specializations do not contain any data member.
		std::atomic<std::size_t> counter;
        std::size_t hash;
		std::size_t size;
		T data[1];

		std::size_t addref(){
			std::size_t old = counter.fetch_add(1);
			XR_PRE_CONDITION(old != 0);
			return old + 1;
		}
		std::size_t release(){
			std::size_t old = counter.fetch_sub(1);
			XR_PRE_CONDITION(old != 0);
			return old - 1;
		}
		std::size_t count(){
			return counter.load();
		}

        void hash_self(){
            hash = 2166136261U;
            uint32_t* p = reinterpret_cast<uint32_t*>(data);
            for (uint32_t* pend = p + size * sizeof(T)/sizeof(uint32_t); p != pend; ++p){
                hash = hash * 16777619U ^ (*p);
            }
            T* pch = reinterpret_cast<T*>(p);
            for(size_t first = 0; first < size % sizeof(uint32_t); ++first){
                hash = 16777619U * hash ^ (size_t)pch[first];
            }
        }
	};
}}
#include <xirang2/config/abi_suffix.h>

#endif //end XR_IMP_SHARED_DATA_H__
