#ifndef XR_COMMON_ARCHIVE_EXT_HEAP_ARCHIVE_H__
#define XR_COMMON_ARCHIVE_EXT_HEAP_ARCHIVE_H__

#include <xirang2/io.h>
#include <xirang2/memory.h>

namespace xirang2{ namespace io{
	struct XR_API ext_heap_archive // reader, writer, random
	{
		typedef reader::iterator iterator;
		typedef writer::const_iterator const_iterator;
		ext_heap_archive(ext_heap& eh, ext_heap::handle h);
		range<iterator> read(const range<iterator>& buf);
		bool readable() const;

		range<const_iterator> write(const range<const_iterator>& r);
		long_size_t truncate(long_size_t size);
		bool writable() const;
		void sync() ;

		long_size_t offset() const;
		long_size_t size() const;
		long_size_t seek(long_size_t offset);

		unique_ptr<read_view> view_rd(ext_heap::handle h);
		unique_ptr<write_view> view_wr(ext_heap::handle h);

        ext_heap::handle get_handle() const;
	private:
		long_size_t m_pos;
		ext_heap& m_heap;
	};

} }

#endif //end XR_COMMON_ARCHIVE_EXT_HEAP_ARCHIVE_H__


