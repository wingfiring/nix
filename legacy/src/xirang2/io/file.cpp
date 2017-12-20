#include <xirang2/io/file.h>
#include <xirang2/to_string.h>

#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/mapped_region.hpp>

#include <boost/numeric/conversion/cast.hpp>
#include <xirang2/string_algo/utf8.h>
#include <xirang2/fsutility.h>


#include <type_traits>  //std::forward

namespace xirang2{ namespace io{

	using namespace boost::interprocess;
	namespace bi = boost::interprocess;
	using boost::numeric_cast;

	static const long_size_t K_buffer_view_size = 256 * 1024;

	struct read_file_view : read_view{
			read_file_view(file_mapping& file, bi::mode_t mode, offset_t offset, std::size_t size)
				: m_size(size)
			{
				if (size > 0)
					m_region = mapped_region(file, mode, offset, size);
			}
			virtual range<const byte*> address() const{
				if (m_size == 0)
					return range<const byte*>();

				const byte* first = reinterpret_cast<const byte*>(m_region.get_address());
				return range<const byte*>(first, first + m_size);
			}
		private:
			mapped_region m_region;
			std::size_t m_size;
	};
	struct write_file_view : write_view{
			write_file_view(file_mapping& file, bi::mode_t mode, offset_t offset, std::size_t size)
				: m_region(file, mode, offset, size)
			{}
			virtual range<byte*> address() const{
				byte* first = reinterpret_cast<byte*>(m_region.get_address());
				return range<byte*>(first, first + m_region.get_size());
			}
		private:
			mapped_region m_region;
	};

	struct file_imp
	{
		typedef reader::iterator iterator;
		typedef writer::const_iterator const_iterator;

		file_imp(const file_path& path, int of, bi::mode_t mode)
			: m_path(path), m_pos(0), m_mode(mode), m_file_size(0), m_flag(of)
		{
			switch (of & of_low_mask)
			{
				case of_open:
					if (!exists_())
						XR_THROW(fs::not_found_exception)(m_path.str());

					break;
				case of_create:
					if (exists_())
						XR_THROW(fs::exist_exception)(m_path.str());
					create_();

					break;
				case of_create_or_open:
					if (!exists_())
						create_();
					break;
				default:
					XR_PRE_CONDITION(false);
			}

			try
			{
#ifdef WIN32
				m_file = file_mapping(m_path.native_wstr().c_str(), mode);
#else
                m_file = file_mapping(m_path.str().c_str(), mode);
#endif
				m_real_file_size  = m_file_size = get_file_size_();
			}
			catch(...)
			{
				XR_THROW(fs::open_failed_exception)(m_path.str());
			}
		}
        ~file_imp()
        {
            if (m_flag & of_remove_on_close)
            {
				file_mapping(std::move(m_file)); //close file_mapping first;
				xirang2::fs::remove(m_path);
            }
            else
            {
                sync();
            }
        }
		range<iterator> read(const range<iterator>& buf)
		{
			XR_PRE_CONDITION(m_pos <= m_file_size);

			long_size_t buf_size = buf.size();
			iterator itr = buf.begin();
			if (buf_size > 0 && readable() )
			{
				long_size_t view_size = std::min(buf_size, m_file_size - m_pos);
				auto new_pos = m_pos + view_size;
				offset_range read_range(m_pos, new_pos);
				if (!m_buffer_range.contains(read_range)){
					m_buffer_range = offset_range(m_pos, std::min(m_pos + std::max(buf_size, K_buffer_view_size), m_real_file_size));
					mapped_region(m_file, m_mode, m_buffer_range.begin(), numeric_cast<size_t>(m_buffer_range.size()))
						.swap(m_buffer_view);
				}

				const byte* psrc = (const byte*)m_buffer_view.get_address() + (m_pos - m_buffer_range.begin());
				std::copy(psrc, psrc + view_size, itr);

				itr += view_size;
				m_pos += view_size;
			}
			return range<iterator>(itr, buf.end());
		}

		bool readable() const { return m_pos < m_file_size; }

		range<const_iterator> write(const range<const_iterator>& r)
		{
			long_size_t buf_size = r.size();
			if (buf_size > 0)
			{
				long_size_t new_pos = m_pos + buf_size;
				if (new_pos > m_real_file_size) // prepare real file size
				{
					auto new_real_size = m_pos + std::max(buf_size, K_buffer_view_size);
					truncate_(new_real_size);
					m_real_file_size = new_real_size;
				}

				//XXX: need more tuning for better performance, avoid to create small view rapidly
				offset_range write_range(m_pos, new_pos);
				if(!m_buffer_range.contains(write_range)){
					m_buffer_range = offset_range(m_pos, new_pos);
					mapped_region(m_file, m_mode, m_buffer_range.begin(), numeric_cast<size_t>(m_buffer_range.size()))
						.swap(m_buffer_view);
				}

				byte* pdest = (byte*)m_buffer_view.get_address() + (m_pos - m_buffer_range.begin());
				std::copy(r.begin(), r.end(), pdest);

				m_pos = new_pos;
                m_file_size = std::max(m_file_size, new_pos);
			}
			return range<const_iterator>(r.end(), r.end());
		}


		void truncate_(long_size_t nsize){
			mapped_region().swap(m_buffer_view);
			m_buffer_range = offset_range();

			if (!boost::interprocess::ipcdetail::truncate_file(m_file.get_mapping_handle().handle, std::size_t(nsize)))
                XR_THROW(archive_append_failed)(to_string(nsize).c_str());
		}
		long_size_t truncate(long_size_t nsize)
		{
			truncate_(nsize);
            m_real_file_size = m_file_size = nsize;
            if (m_pos > m_file_size)
                m_pos = m_file_size;

            return m_file_size;
		}
		bool writable() const { return true;}
		void sync()
		{
			if (m_real_file_size != m_file_size){
				truncate_(m_file_size);
				m_real_file_size = m_file_size;
			}
			//TODO: imp sync
		}
		unique_ptr<write_view> view_wr(ext_heap::handle h)
		{
			XR_PRE_CONDITION(h.begin() >= 0);
			if (m_file_size < numeric_cast<long_size_t>(h.end()))
				truncate(h.end());

			return unique_ptr<write_view>(new write_file_view(m_file, m_mode, h.begin(), numeric_cast<std::size_t>(h.size())));
		}

		unique_ptr<read_view> view_rd(ext_heap::handle h)
		{
			XR_PRE_CONDITION(h.begin() >= 0);
			XR_PRE_CONDITION(h.empty() || numeric_cast<long_size_t>(h.begin()) < m_file_size);

			if (m_file_size < numeric_cast<long_size_t>(h.end()) )
				h = ext_heap::handle(h.begin(), m_file_size);

			return unique_ptr<read_view>(new read_file_view(m_file, m_mode, h.begin(), numeric_cast<std::size_t>(h.size())));
		}

		long_size_t offset() const { return m_pos; }
		long_size_t size() const { return m_file_size;}
		long_size_t seek(long_size_t offset)
		{
			m_pos = offset;
			return m_pos;
		}

        private:
		bool exists_()
		{

			return fs::exists(m_path);
		}

		void create_()
		{
#ifdef WIN32_OS_
            wstring wpath = m_path.native_wstr();
			FILE* fp = _wfopen(wpath.c_str(), L"wb");
#else
            FILE* fp = fopen(m_path.str().c_str(),"wb");
#endif
			if (fp == 0)
				XR_THROW(fs::create_exception)(m_path.str());

			fclose(fp);
		}

		long_size_t get_file_size_()
		{
			boost::interprocess::offset_t size = 0;
			if (!boost::interprocess::ipcdetail::get_file_size(m_file.get_mapping_handle().handle, size))
				XR_THROW(fs::not_found_exception)(m_path.str());
			return  long_size_t(size);
		}


		file_path m_path;
		long_size_t m_pos;
		bi::mode_t m_mode;
		long_size_t m_file_size;
		file_mapping m_file;
        int m_flag;
		mapped_region m_buffer_view;
		ext_heap::handle m_buffer_range;
		long_size_t m_real_file_size;
	};

	/////////////////////////////////////////////////
	file_reader::file_reader(const file_path& path)
		: m_imp(new file_imp(path, of_open, read_only))
	{
	}
    file_reader::file_reader(file_reader&& rhs)
        : m_imp(rhs.m_imp)
    {
        rhs.m_imp = nullptr;
    }
    
    file_reader& file_reader::operator=(file_reader&& rhs){
        file_reader(std::forward<file_reader>(rhs)).swap(*this);
        return *this;
    }
	file_reader::~file_reader() { check_delete(m_imp);}
    void file_reader::swap(file_reader& rhs){
        std::swap(m_imp, rhs.m_imp);
    }

	range<file_reader::iterator> file_reader::read(const range<file_reader::iterator>& buf)
	{
		return m_imp->read(buf);
	}
	bool file_reader::readable() const { return m_imp->readable();}
	unique_ptr<read_view> file_reader::view_rd(ext_heap::handle h) const { return m_imp->view_rd(h);}

	long_size_t file_reader::offset() const { return m_imp->offset();}
	long_size_t file_reader::size() const	{ return m_imp->size();}
	long_size_t file_reader::seek(long_size_t offset) {
		if(offset > size()) offset = size();
		return m_imp->seek(offset);
	}

	/////////////////////////////////////////////////

	file_writer::file_writer(const file_path& path,  int of)
		: m_imp(new file_imp(path, of, read_write))
	{}

	file_writer::~file_writer() 	{ check_delete(m_imp);}
    
    file_writer::file_writer(file_writer&& rhs)
        : m_imp(rhs.m_imp)
    {
        rhs.m_imp = nullptr;
    }
    
    file_writer& file_writer::operator=(file_writer&& rhs){
        file_writer(std::forward<file_writer>(rhs)).swap(*this);
        return *this;
    }
    
    void file_writer::swap(file_writer& rhs){
        std::swap(m_imp, rhs.m_imp);
    }

	range<file_writer::const_iterator> file_writer::write(
			const range<file_writer::const_iterator>& r)
	{ return m_imp->write(r);}
	long_size_t file_writer::truncate(long_size_t size)	{ return m_imp->truncate(size);}
	bool file_writer::writable() const	{ return m_imp->writable(); }
	void file_writer::sync() 	{ m_imp->sync(); }
	unique_ptr<write_view> file_writer::view_wr(ext_heap::handle h){ return m_imp->view_wr(h);}

	long_size_t file_writer::offset() const	{ return m_imp->offset(); }
	long_size_t file_writer::size() const		{ return m_imp->size(); }
	long_size_t file_writer::seek(long_size_t offset) { return m_imp->seek(offset); }

	/////////////////////////////////////////////////

	file::file(const file_path& path, int of)
		: m_imp(new file_imp(path, of, read_write))
	{}
	file::~file()	{ check_delete(m_imp);}

    file::file(file&& rhs)
        : m_imp(rhs.m_imp)
    {
        rhs.m_imp = nullptr;
    }
    
    file& file::operator=(file&& rhs){
        file(std::forward<file>(rhs)).swap(*this);
        return *this;
    }
    
    void file::swap(file& rhs){
        std::swap(m_imp, rhs.m_imp);
    }
    
	range<file::iterator> file::read(
			const range<file::iterator>& buf)
	{	return m_imp->read(buf);	}

	bool file::readable() const	{ return m_imp->readable(); }
	unique_ptr<read_view> file::view_rd(ext_heap::handle h) const { return m_imp->view_rd(h);}
	range<file::const_iterator> file::write(
			const range<file::const_iterator>& r)
	{ return m_imp->write(r); }

	long_size_t file::truncate(long_size_t size)	{ return m_imp->truncate(size);}
	bool file::writable() const	{ return m_imp->writable(); }
	void file::sync() { m_imp->sync();}
	unique_ptr<write_view> file::view_wr(ext_heap::handle h){ return m_imp->view_wr(h);}

	long_size_t file::offset() const	{ return m_imp->offset(); }
	long_size_t file::size() const	{ return m_imp->size();}
	long_size_t file::seek(long_size_t offset)	{ return m_imp->seek(offset);}
} }

