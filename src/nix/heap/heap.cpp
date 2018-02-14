#include <nix/memory/heap.h>

#include <typeinfo>
namespace nix
{
	plain_heap::~plain_heap(){}

	plain_heap::plain_heap()
	{}

	/// call platform malloc. the parameter alignment and hint are ignored.
	void* plain_heap::malloc(std::size_t size, std::size_t /* alignment */, const void* /* hint */)
	{
		return ::operator new(size);
	}

	/// call platform free. the parameter alignment is ignored.
	void plain_heap::free(void* p, std::size_t /* size */, std::size_t /* alignment */ ) noexcept
	{
		::operator delete(p);
	}

	/// return null for always.
	heap* plain_heap::underling() const noexcept
	{
		return 0;
	}

	bool plain_heap::equal_to(const heap& rhs) const noexcept
	{
		return typeid(*this) == typeid(rhs);
	}
}
