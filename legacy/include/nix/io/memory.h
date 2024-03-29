#ifndef XR_COMMON_ARCHIVE_MEM_ARCHIVE_H__
#define XR_COMMON_ARCHIVE_MEM_ARCHIVE_H__

#include <xirang2/io.h>

namespace xirang2{ namespace io{

	struct XR_API buffer_rd_view : read_view
	{
		range<const byte*> address() const { return m_data;}

		buffer_rd_view() : m_data(){}
		explicit buffer_rd_view(range<const byte*> data) : m_data(data){}
		private:
		range<const byte*> m_data;
	};

	struct XR_API buffer_wr_view : write_view
	{
		range<byte*> address() const { return m_data;}

		buffer_wr_view() : m_data(){}
		explicit buffer_wr_view(range<byte*> data) : m_data(data){}
		private:
		range<byte*> m_data;
	};

	struct XR_API buffer_in	// reader, random, const_view
	{
		typedef byte* iterator;
		explicit buffer_in(const buffer<byte>& buf);
		explicit buffer_in(const range<const byte*>& buf);

		range<byte*> read(const range<byte*>& buf);
		bool readable() const;

		long_size_t offset() const;
		long_size_t size() const;
		long_size_t seek(long_size_t offset);

		buffer_rd_view view_rd(ext_heap::handle h) const;

		range<const byte*> data() const;
	private:
		long_size_t m_pos;
		range<const byte*> m_data;

	};

	struct XR_API buffer_out	//  writer, random, view
	{
		typedef const byte* const_iterator;
		explicit buffer_out(buffer<byte>& buf);

		range<const byte*> write(const range<const byte*>& r);
		long_size_t truncate(long_size_t size);
		bool writable() const;
		void sync() ;

		long_size_t offset() const;
		long_size_t size() const;
		long_size_t seek(long_size_t offset);

		buffer_wr_view view_wr(ext_heap::handle h);
		buffer<byte> & data();
		const buffer<byte> & data() const;
	protected:
		size_t m_pos;
		buffer<byte>* m_data;
	};

	struct XR_API buffer_io // reader, writer, random, const_view, view
	{
		typedef reader::iterator iterator;
		typedef writer::const_iterator const_iterator;

		explicit buffer_io(buffer<byte>& buf);
		range<iterator> read(const range<byte*>& buf);
		bool readable() const;

		range<const byte*> write(const range<const byte*>& r);
		long_size_t truncate(long_size_t size);
		bool writable() const;
		void sync() ;

		long_size_t offset() const;
		long_size_t size() const;
		long_size_t seek(long_size_t offset);

		buffer_rd_view view_rd(ext_heap::handle h) const;
		buffer_wr_view view_wr(ext_heap::handle h);

		buffer<byte> & data();
		const buffer<byte> & data() const;

		protected:
		size_t m_pos;
		buffer<byte>* m_data;
	};

	struct XR_API fixed_buffer_io	//  writer, random, view
	{
		typedef byte* iterator;
		typedef const byte* const_iterator;
		explicit fixed_buffer_io(const range<byte*>& buf);

		range<byte*> read(const range<byte*>& buf);
		bool readable() const;

		range<const byte*> write(const range<const byte*>& r);
		bool writable() const;
		void sync();

		long_size_t offset() const;
		long_size_t size() const;
		long_size_t seek(long_size_t offset);

		long_size_t truncate(long_size_t size);
		buffer_wr_view view_wr(ext_heap::handle h);
		buffer_rd_view view_rd(ext_heap::handle h) const;
		range<byte*> data() const;
	private:
		size_t m_pos;
		range<byte*> m_data;
	};

	typedef buffer_in mem_reader;

	struct XR_API mem_writer : buffer_out
	{
		mem_writer();
        mem_writer(const mem_writer&);
        mem_writer(mem_writer&&);
        
        mem_writer& operator=(const mem_writer&);
        mem_writer& operator=(mem_writer&&);
        
		private:
		buffer<byte> m_data;
	};

	struct XR_API mem_archive : buffer_io
	{
		mem_archive();
        mem_archive(mem_archive&&);
        mem_archive(const mem_archive&);
		explicit mem_archive(const buffer<byte>& buf);
        
        mem_archive& operator=(const mem_archive&);
        mem_archive& operator=(mem_archive&&);
        
		private:
		buffer<byte> m_data;
	};

	struct XR_API zero{
		zero();
		range<byte*> read(const range<byte*>& buf);
		bool readable() const;

		long_size_t offset() const;
		long_size_t size() const;
		long_size_t seek(long_size_t offset);
		private:
		long_size_t m_pos;
	};
	struct XR_API null{
		null();
		range<const byte*> write(const range<const byte*>& r);
		bool writable() const;
		void sync() ;

		long_size_t offset() const;
		long_size_t size() const;
		long_size_t seek(long_size_t offset);

		long_size_t truncate(long_size_t size);
		private:
		long_size_t m_pos;
	};

	struct XR_API empty{
		range<byte*> read(const range<byte*>& buf);
		bool readable() const;

		long_size_t offset() const;
		long_size_t size() const;
		long_size_t seek(long_size_t offset);
	};

}}

#endif //end XR_COMMON_ARCHIVE_MEM_ARCHIVE_H__


