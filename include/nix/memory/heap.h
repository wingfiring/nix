//NIX_LICENSE_PLACE_HOLDER

#ifndef NIX_MEMORY_HEAP_H__
#define NIX_MEMORY_HEAP_H__
#include <nix/offset_range.h>

namespace nix
{
	/// define the thread poliy enumeration
	enum thread_policy
	{
		single_thread,	///< indicate single thread memory allocation
		multi_thread	///< multi threaded memory allocation
	};

	/// heap interface
	class NIX_INTERFACE heap
	{
	public:
		/// allocate a memory blcok with specified size
		/// \param size bytes of required block
		/// \param alignment the alignment of allocated memory block. The minimal is 1, alignment must be 2^N
		/// \param hint hint of allocation, it's used to improve the performance for some implementation. nullptr means no hint.
		/// \return head address of allocated memory blcok
		/// \pre size >= 0
		/// \post return != nullptr
		/// \throw std::bad_alloc
		virtual void* malloc(std::size_t size, std::size_t alignment, const void* hint ) = 0;

		/// free the memory block allocated by current handler instance or equivalent
		/// \param p head address of memory block
		/// \param alignment the alignment of allocated memory block. Default is 1, alignment must be 2^N, 0 means unknown
		/// \param size size of memory block. if size equals to 0, means unknown size and heap imp should check the size. The check may be slow.
		/// \pre size and alignment should match the malloc call or 0, and p should belong to this heap
		/// \throw nothrow
		/// \note if p is null, do nothing.
		virtual void free(void* p, std::size_t size, std::size_t alignment ) noexcept = 0;

		/// query the underling heap
		/// \return return the underling heap, can be null.
		virtual heap* underling() const noexcept= 0;

		/// return true if two memory handlers are equivalent
		/// \param rhs the other memory handler reference
		/// \return true if equal otherwise false
		/// \throw nothrow
		virtual bool equal_to(const heap& rhs) const noexcept = 0;

		/// \dtor
		virtual ~heap(){};
	};

	/// plain heap. it forward the malloc and free to platform call directly.
	/// notes: it would be used as default global heap, so it must be implemented and defend re-construct & re-destruct
	class NIX_API plain_heap : public heap
	{
	public:
		/// ctor 
		/// \param thp this parameter is ignored.
		explicit plain_heap();

		virtual ~plain_heap();

		/// call platform malloc. the parameter alignment and hint are ignored.
		virtual void* malloc(std::size_t size, std::size_t alignment, const void* hint);

		/// call platform free. the parameter alignment is ignored.
		virtual void free(void* p, std::size_t size, std::size_t alignment) noexcept ;

		/// return null for always.
		virtual heap* underling() const noexcept;

		virtual bool equal_to(const heap& rhs) const noexcept;
	};

#if 1		//TODO: not implemented
	class NIX_API pool_heap : heap
	{
	public:
		explicit pool_heap(heap* under, thread_policy thp);
		virtual ~pool_heap();

		virtual void* malloc(std::size_t size, std::size_t alignment, const void* hint);
		virtual void free(void* p, std::size_t size, std::size_t alignment) noexcept;
		virtual heap* underling() const noexcept;

		virtual bool equal_to(const heap& rhs) const noexcept;

		virtual void prepare_destroy();
	};
#endif

	class NIX_INTERFACE ext_heap : public heap
	{
	public: //heap methods
		typedef offset_range handle;
		virtual void* malloc(std::size_t size, std::size_t alignment, const void* hint ) = 0;

		virtual void free(void* p, std::size_t size, std::size_t alignment ) noexcept = 0;

		/// \return the underling ext_heap, can be null.
		virtual heap* underling() const noexcept = 0;

		virtual bool equal_to(const heap& rhs) const noexcept = 0;

	public:

		/// allocate a block in external heap
		virtual handle allocate(std::size_t size, std::size_t alignment, handle hint) = 0;

		/// release an external block
		virtual void deallocate(handle p) noexcept = 0;

		/// map a block into memory, ref count internally.
		virtual void* track_pin(handle h) = 0;
		/// map a block into memory, just track the view, not handle
		virtual void* pin(handle h) = 0;

		virtual int track_pin_count(handle h) const noexcept = 0;
		virtual int view_pin_count(handle h) const noexcept = 0;

		/// unmap a block.
		virtual int track_unpin(void* h) = 0;
		/// unmap a block.
		virtual int unpin(void* h) noexcept = 0;

		/// TODO: is it necessary?
		/// write to external block directly. if h have been mapped into memory, update the memory.
		virtual std::size_t write(handle h, const void* src, std::size_t n) = 0;

		/// TODO: is it necessary?
		/// read from external block directly. if the block has been mapped, read the memory block.
		virtual std::size_t read(handle, void* dest, std::size_t) = 0;

		/// sync the memory to external, if h is invalid, sync all.
		virtual void sync(handle h) = 0;

	protected:
		virtual ~ext_heap(){};
	};
	/// return the global heap.
	NIX_API extern heap& get_global_heap();

}

#endif //end NIX_MEMORY_HEAP_H__

