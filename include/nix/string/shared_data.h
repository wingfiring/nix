#ifndef NIX_IMP_SHARED_DATA_H__
#define NIX_IMP_SHARED_DATA_H__

#include <nix/config.h>

#include <atomic>
namespace nix{ 
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

		std::size_t addref() noexcept{
			std::size_t old = counter.fetch_add(1);
			NIX_EXPECTS(old != 0);
			return old + 1;
		}
		std::size_t release() noexcept{
			std::size_t old = counter.fetch_sub(1);
			NIX_EXPECTS(old != 0);
			return old - 1;
		}
		std::size_t count() const noexcept{
			return counter.load();
		}
	};
}}

#endif //end NIX_IMP_SHARED_DATA_H__
