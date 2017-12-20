#ifndef XR_COMMON_ZIP_H__
#define XR_COMMON_ZIP_H__

#include <xirang2/io.h>

namespace xirang2{ namespace zip{

	typedef range<const byte*> dict_type;

	///\@see coreponding in zlib
	enum level{
		zl_default = -1,
		zl_no_compression = 0,
		zl_best_speed = 1,
		zl_best_compression = 9
	};

	enum strategy{
		zs_filtered = 1,
		zs_huffman_only = 2,
		zs_default = 0
	};

	enum error{
		ze_ok,
		ze_internal_error,
		ze_data_error,
		ze_mem_error,
		ze_need_dict,
		ze_stream_error,
		ze_unfinished_data,
		ze_crc_error
	};

	struct zip_result{
		int err;
		long_size_t in_size;
		long_size_t out_size;
	};

	enum zip_format{
		zm_raw_deflate,
		zm_gzip,
		zm_zlib,
		zm_inflate_gzip_or_zlib,
	};

	XR_EXCEPTION_TYPE(inflate_exception);
	XR_EXCEPTION_TYPE(deflate_exception);

	/// return crc32 initilaized value
	XR_API extern uint32_t crc32_init();

	//caculate new crc from input
	XR_API extern uint32_t crc32(io::reader& src, uint32_t crc = crc32_init());
	XR_API extern uint32_t crc32(io::read_map& src, uint32_t crc = crc32_init());
	XR_API extern uint32_t crc32(range<const byte*> src, uint32_t crc = crc32_init());

	XR_API extern zip_result inflate(io::reader& src, io::writer& dest, zip_format format = zm_raw_deflate, dict_type dict = dict_type(), heap* h = 0);
	XR_API extern zip_result inflate(io::read_map& src, io::write_map& dest, zip_format format = zm_raw_deflate, dict_type dict = dict_type(), heap* h = 0);

	XR_API extern zip_result deflate(io::reader& src, io::writer& dest, zip_format format = zm_raw_deflate, int level = zl_default, dict_type dict = dict_type(), heap* h = 0, int strategy_ = zs_default);
	XR_API extern zip_result deflate(io::read_map& src, io::write_map& dest, zip_format format = zm_raw_deflate, int level = zl_default, dict_type dict = dict_type(), heap* h = 0, int strategy_ = zs_default);

	// imp reader, forward
	class inflate_reader_imp;
	class XR_API inflate_reader{
		public:
			inflate_reader();
			~inflate_reader();

			inflate_reader(inflate_reader&& rhs);
			inflate_reader& operator=(inflate_reader&& rhs);
			void swap(inflate_reader& rhs);

			explicit inflate_reader(io::reader& src, zip_format format = zm_raw_deflate, long_size_t uncompressed_size = long_size_t(-1),dict_type dict = dict_type(),heap* h = 0);
			explicit inflate_reader(io::read_map& src, zip_format format = zm_raw_deflate, long_size_t uncompressed_size = long_size_t(-1),dict_type dict = dict_type(),heap* h = 0);
			bool valid() const;
			explicit operator bool() const;

			range<byte*> read(const range<byte*>& buf);
			bool readable() const;
			long_size_t offset() const;
			long_size_t size() const;
			long_size_t seek(long_size_t off);

			long_size_t compressed_size() const;
			uint32_t crc32() const;

			inflate_reader(const inflate_reader&) = delete;
			inflate_reader& operator=(const inflate_reader&) = delete;
		private:
			unique_ptr<inflate_reader_imp> m_imp;
	};
	//TODO:
	class inflate_writer;

	// imp writer, sequence
	class deflate_writer_imp;
	class XR_API deflate_writer{
		public:
			deflate_writer();
			~deflate_writer();
			deflate_writer(deflate_writer&& rhs);
			deflate_writer& operator=(deflate_writer&& rhs);
			void swap(deflate_writer& rhs);

			explicit deflate_writer(io::writer& dest, zip_format format = zm_raw_deflate,
					int level = zl_default, dict_type dict = dict_type(),
					heap* h = 0, int strategy_ = zs_default);
			explicit deflate_writer(io::write_map& dest, zip_format format = zm_raw_deflate,
					int level = zl_default, dict_type dict = dict_type(),
					heap* h = 0, int strategy_ = zs_default);
			bool valid() const;
			explicit operator bool() const;

			range<const byte*> write(const range<const byte*>& buf);
			bool writable() const;
			long_size_t offset() const;
			long_size_t size() const;
			void sync();

			long_size_t uncompressed_size() const;
			uint32_t crc32() const;
			void finish() ;
			bool finished() const ;

			deflate_writer(const deflate_writer& rhs) = delete;
			deflate_writer& operator=(const deflate_writer&) = delete;
		private:
			unique_ptr<deflate_writer_imp> m_imp;
	};

	//TODO:
	class deflate_reader;

}}

#endif //end XR_COMMON_ZIP_H__


