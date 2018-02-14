#include <nix/memory.h>
#include <atomic>
#include <cstddef>

namespace nix
{
	namespace
	{
		plain_heap g_default_heap;
		// NOTE: when DSO loaded, the memory g_default_heap_init_flag resident is filled by zero, 
		// once g_default_heap initialized, g_default_heap_init_flag would be constructed with "initialized".
		// so, if g_default_heap_init_flag.empty() is true, we know the g_default_heap is still not constructed.
		std::string g_default_heap_init_flag("initialized");
	}

	NIX_API heap& get_global_heap() {
		if (g_default_heap_init_flag.empty()){	// empty means it's not initialized
			new (&g_default_heap) plain_heap;	// NOTE: plain_heap is ready for multiple constructions
		}
		return g_default_heap;
	}

	NIX_API extern std::size_t max_alignment()
	{
		return alignof(std::max_align_t);
	}

}

