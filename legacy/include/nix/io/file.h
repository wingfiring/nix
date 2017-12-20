#ifndef XR_COMMON_ARCHIVE_FILE_ARCHIVE_H__
#define XR_COMMON_ARCHIVE_FILE_ARCHIVE_H__

#include <xirang2/io.h>
#include <xirang2/memory.h>
#include <xirang2/path.h>

namespace xirang2{ namespace io{
	struct file_imp;

	struct XR_API file_reader // <reader, random >
	{
		typedef reader::iterator iterator;

		explicit file_reader(const file_path& path);
        file_reader(file_reader&& rhs);
		~file_reader();

		range<iterator> read(const range<iterator>& buf);
		bool readable() const;

		long_size_t offset() const;
		long_size_t size() const;
		long_size_t seek(long_size_t offset);

		unique_ptr<read_view> view_rd(ext_heap::handle h) const;
        
        file_reader& operator=(file_reader&& rhs);
        void swap(file_reader& rhs);
    private:
        file_reader(const file_reader&);    //disabled
        file_reader& operator=(const file_reader&); //disabled
		file_imp * m_imp;;
	};

	XR_EXCEPTION_TYPE(archive_append_failed);

	struct XR_API file_writer // <writer, random >
	{
		typedef writer::const_iterator const_iterator;
		typedef writer::const_iterator iterator;

		explicit file_writer(const file_path& path,  int of);
        file_writer(file_writer&& rhs);
		~file_writer();

		range<const_iterator> write(const range<const_iterator>& r);
		long_size_t truncate(long_size_t size);
		bool writable() const;
		void sync() ;

		long_size_t offset() const;
		long_size_t size() const;
		long_size_t seek(long_size_t offset);

		unique_ptr<write_view> view_wr(ext_heap::handle h);
        
        file_writer& operator=(file_writer&& rhs);
        void swap(file_writer& rhs);
    private:
        file_writer(const file_writer&);        //disabled
        file_writer& operator=(const file_writer&);     //disabled
		file_imp * m_imp;;
	};

	struct XR_API file // <reader, writer, random >
	{
		typedef reader::iterator iterator;
		typedef writer::const_iterator const_iterator;

		explicit file(const file_path& path, int of);
        file(file&& rhs);
		~file();

		range<iterator> read(const range<iterator>& buf);
		bool readable() const;

		range<const_iterator> write(const range<const_iterator>& r);
		long_size_t truncate(long_size_t size);
		bool writable() const;
		void sync() ;

		long_size_t offset() const;
		long_size_t size() const;
		long_size_t seek(long_size_t offset);

		unique_ptr<read_view> view_rd(ext_heap::handle h) const;
		unique_ptr<write_view> view_wr(ext_heap::handle h);
        
        file& operator=(file&& rhs);
        void swap(file& rhs);
	private:
        file(const file&);
        file& operator=(const file&);
		file_imp * m_imp;;
	};


}}

#endif //end XR_COMMON_ARCHIVE_FILE_ARCHIVE_H__

