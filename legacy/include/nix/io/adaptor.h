#ifndef XR_COMMON_ARCHIVE_ADAPTOR_H__
#define XR_COMMON_ARCHIVE_ADAPTOR_H__

#include <xirang2/io.h>
#include <xirang2/io/memory.h>
#include <functional>

namespace xirang2{ namespace io{
	namespace io_private_{
		template<typename Interface, typename Archive> typename std::enable_if<!is_iref<Archive>::value, Archive&>::type get_i_(Archive& ar){
			return ar;
		}
		template<typename Interface, typename Archive>
			const typename std::enable_if<!is_iref<Archive>::value, const Archive&>::type get_i_(const Archive& ar){
			return ar;
		}
		template<typename Interface, typename ...Interfaces> Interface& get_i_(const iref<Interfaces...>& ar){
			return ar.template get<Interface>();
		}
		template<typename Interface, typename ...Interfaces> Interface& get_i_(const iauto<Interfaces...>& ar){
			return ar.template get<Interface>();
		}

		template<typename Interface, typename Archive> typename std::enable_if<!is_iref<Archive>::value, Archive&>::type get_i_(Archive* ar){
			return *ar;
		}
		template<typename Interface, typename Archive>
			const typename std::enable_if<!is_iref<Archive>::value, const Archive&>::type get_i_(const Archive* ar){
			return *ar;
		}
		template<typename Interface, typename ...Interfaces> Interface& get_i_(const iref<Interfaces...>* ar){
			return ar->template get<Interface>();
		}
		template<typename Interface, typename ...Interfaces> Interface& get_i_(const iauto<Interfaces...>* ar){
			return ar->template get<Interface>();
		}
	}

	template<typename ArchiveData, template<typename> class ... PartialInterfaces>
		struct decorator : public ArchiveData,
		public PartialInterfaces<decorator<ArchiveData, PartialInterfaces...> >...
	{
		typedef typename ArchiveData::archive_type archive_type;
		template<typename ... Args>
		explicit decorator(Args&& ... args) : ArchiveData(std::forward<Args>(args)...){}
        decorator(decorator&& rhs) : ArchiveData(std::forward<ArchiveData>(rhs)){}
	};

	template<typename Decorator> struct get_archive_type;

	template<typename ArchiveData, template<typename> class ... PartialInterfaces>
	struct get_archive_type<decorator<ArchiveData, PartialInterfaces...>>{
		typedef typename ArchiveData::archive_type type;
	};

	template<typename ArchiveData, template<typename>class... PartialInterfaces, typename Ar, typename... T>
		decorator<ArchiveData, PartialInterfaces...> decorate2(Ar&& ar, T&&... args){
			return decorator<ArchiveData, PartialInterfaces...>(std::forward<Ar>(ar), std::forward<T>(args)...);
		}
	template<template<typename> class ArchiveData, template<typename>class... PartialInterfaces, typename Ar, typename... T>
		decorator<ArchiveData<typename std::conditional<std::is_pointer<Ar&&>::value || std::is_rvalue_reference<Ar&&>::value,
				   typename std::remove_reference<Ar>::type,
				   typename std::add_lvalue_reference<Ar>::type
					   >::type>,
				   PartialInterfaces...>
		decorate(Ar&& ar, T&&... args){
			return decorator<ArchiveData<typename std::conditional<std::is_pointer<Ar&&>::value || std::is_rvalue_reference<Ar&&>::value,
				   typename std::remove_reference<Ar>::type,
				   typename std::add_lvalue_reference<Ar>::type
					   >::type>,
				   PartialInterfaces...>(std::forward<Ar>(ar), std::forward<T>(args)...);
		}

#define COMMON_IO_ADAPTOR_HELPER()\
		const Derive& derive_() const{ return static_cast<const Derive&>(*this);}\
		Derive& derive_() { return static_cast<Derive&>(*this);}\
		typedef typename get_archive_type<Derive>::type archive_type_;\
		template<typename Interface> auto underlying_() const \
			->decltype(io_private_::get_i_<Interface>(*static_cast<const archive_type_*>(nullptr))) { \
			return io_private_::get_i_<Interface>(derive_().underlying());\
		}\
		template<typename Interface> auto underlying_() \
			->decltype(io_private_::get_i_<Interface>(*static_cast<archive_type_*>(nullptr))) { \
			return io_private_::get_i_<Interface>(derive_().underlying());\
		}

	template<typename ArchiveType> struct proxy_archive
	{
		typedef typename std::remove_reference<ArchiveType>::type	archive_type;

		proxy_archive(): m_underlying(){}
		template<typename RealArchiveType>
		explicit proxy_archive(RealArchiveType&& ar): m_underlying(std::forward<RealArchiveType>(ar)){}
		proxy_archive(proxy_archive&& rhs) : m_underlying(std::forward<ArchiveType>(rhs.m_underlying)){}
		archive_type& underlying() {
			return m_underlying;
		}
		const archive_type& underlying() const{
			return m_underlying;
		}

		ArchiveType m_underlying;
	};

	template<typename Derive> struct proxy_reader_p : reader
	{
		typedef typename reader::iterator iterator;

		virtual range<iterator> read(const range<iterator>& buf) {
			return underlying_<reader>().read(buf);
		}

		virtual bool readable() const {
			return underlying_<reader>().readable();
		}

		private:
		COMMON_IO_ADAPTOR_HELPER();
	};

	template<typename Derive> struct proxy_writer_p : writer
	{
		typedef typename writer::iterator iterator;
		typedef typename writer::const_iterator const_iterator;

		virtual range<const_iterator> write(const range<const_iterator>& r){
			return underlying_<writer>().write(r);
		}
		virtual bool writable() const{
			return underlying_<writer>().writable();
		}
		virtual void sync(){
			underlying_<writer>().sync();
		}
		private:
		COMMON_IO_ADAPTOR_HELPER();
	};

	template<typename Derive> struct proxy_ioctrl_p : ioctrl
	{
		virtual long_size_t truncate(long_size_t size){
			return underlying_<ioctrl>().truncate(size);
		}
		private:
		COMMON_IO_ADAPTOR_HELPER();
	};

	template<typename Derive> struct proxy_ioinfo_p : ioinfo
	{
		virtual long_size_t size() const{
			return underlying_<ioinfo>().size();
		}
		private:
		COMMON_IO_ADAPTOR_HELPER();
	};

	template<typename Derive> struct proxy_sequence_p : sequence
	{
		virtual long_size_t offset() const{
			return underlying_<sequence>().offset();
		}
		virtual long_size_t size() const{
			return underlying_<sequence>().size();
		}
		private:
		COMMON_IO_ADAPTOR_HELPER();
	};

	template<typename Derive> struct proxy_forward_p : forward
	{
		virtual long_size_t offset() const{
			return underlying_<forward>().offset();
		}
		virtual long_size_t size() const{
			return underlying_<forward>().size();
		}
		virtual long_size_t seek(long_size_t off){
			return underlying_<forward>().seek(off);
		}
		private:
		COMMON_IO_ADAPTOR_HELPER();
	};

	template<typename Derive> struct proxy_random_p : random
	{
		virtual long_size_t offset() const{
			return underlying_<random>().offset();
		}
		virtual long_size_t size() const{
			return underlying_<random>().size();
		}
		virtual long_size_t seek(long_size_t off){
			return underlying_<random>().seek(off);
		}
		private:
		COMMON_IO_ADAPTOR_HELPER();
	};

	template<typename Derive> struct proxy_options_p : options
	{
		virtual any getopt(int id, const any & optdata = any() ) const {
			return underlying_<options>().getopt(id, optdata);
		}
		virtual any setopt(int id, const any & optdata,  const any & indata= any()){
			return underlying_<options>().setopt(id, optdata, indata);
		}
		private:
		COMMON_IO_ADAPTOR_HELPER();
	};

	template<typename Derive> struct proxy_read_view_p : read_view
	{
		virtual range<const byte*> address() const{
			return underlying_<read_view>().address();
		}
		private:
		COMMON_IO_ADAPTOR_HELPER();
	};

	template<typename Derive> struct proxy_write_view_p : write_view
	{
		virtual range<byte*> address() const{
			return underlying_<write_view>().address();
		}
		private:
		COMMON_IO_ADAPTOR_HELPER();
	};

	template<typename Derive> struct proxy_read_map_p : read_map
	{
		virtual iauto<read_view> view_rd(ext_heap::handle h){
			return underlying_<read_map>().view_rd(h);
		}
		virtual long_size_t size() const{
			return underlying_<read_map>().size();
		}
		private:
		COMMON_IO_ADAPTOR_HELPER();
	};

	template<typename Derive> struct proxy_write_map_p : write_map
	{
		virtual iauto<write_view> view_wr(ext_heap::handle h) {
			return underlying_<write_map>().view_wr(h);
		}
		virtual long_size_t size() const{
			return underlying_<write_map>().size();
		}
		virtual void sync() {
			underlying_<write_map>().sync();
		}
		virtual long_size_t truncate(long_size_t s){
			return underlying_<write_map>().truncate(s);
		}
		private:
		COMMON_IO_ADAPTOR_HELPER();
	};

	template<typename ArchiveType>
	struct multiplex_archive : public proxy_archive<ArchiveType>
	{
		typedef proxy_archive<ArchiveType> base;
		multiplex_archive(): current(0){}
		template<typename RealArchiveType>
		multiplex_archive(RealArchiveType&& ar, long_size_t off)
			: base(std::forward<RealArchiveType>(ar)), current(off)
		{}
        multiplex_archive(multiplex_archive&& rhs)
            : proxy_archive<ArchiveType>(std::forward<proxy_archive<ArchiveType> >(rhs)){}
		long_size_t current;
	};


	template<typename Archive>
    struct multiplex_offset_tracker
    {
        Archive& m_ar;

        multiplex_offset_tracker(Archive& ar) : m_ar(ar) {
			io_private_::get_i_<io::random>(m_ar.underlying()).seek(m_ar.current);
        }
        ~multiplex_offset_tracker() {
			m_ar.current = io_private_::get_i_<io::random>(m_ar.underlying()).offset();
        }
    };

	template<typename Derive> struct multiplex_reader_p : reader
	{
		typedef reader::iterator iterator;

		virtual range<iterator> read(const range<iterator>& buf) {
            multiplex_offset_tracker<Derive> tracker(derive_());
			return underlying_<reader>().read(buf);
		}

		virtual bool readable() const {
			auto self = const_cast<multiplex_reader_p*>(this);
			self->template underlying_<io::random>().seek(derive_().current);
			return underlying_<reader>().readable();
		}

		private:
		COMMON_IO_ADAPTOR_HELPER();
	};

	template<typename Derive> struct multiplex_writer_p : writer
	{
		typedef typename writer::iterator iterator;
		typedef typename writer::const_iterator const_iterator;

		virtual range<const_iterator> write(const range<const_iterator>& r){
            multiplex_offset_tracker<Derive> tracker(derive_());
			return underlying_<writer>().write(r);
		}
		virtual bool writable() const{
			auto self = const_cast<multiplex_writer_p*>(this);
			self->template underlying_<random>().seek(derive_().current);
			return underlying_<writer>().writable();
		}
		virtual void sync(){
			underlying_<writer>().sync();
		}
		private:
		COMMON_IO_ADAPTOR_HELPER();
	};

	template<typename Derive> struct multiplex_random_p : random
	{
		virtual long_size_t offset() const {
			return derive_().current;
		}
		virtual long_size_t size() const {
			return underlying_<random>().size();
		}
		virtual long_size_t seek(long_size_t current) {
			return (derive_().current = underlying_<random>().seek(current));
		}
		private:
		COMMON_IO_ADAPTOR_HELPER();
	};

	template<typename Derive> struct multiplex_ioctrl_p : ioctrl
	{
		virtual long_size_t truncate(long_size_t size){
			auto new_size = underlying_<ioctrl>().truncate(size);
			derive_().current = std::min(
					derive_().current,
					new_size);
			return new_size;
		}
		private:
		COMMON_IO_ADAPTOR_HELPER();
	};

	template<typename Derive> using multiplex_options_p = proxy_options_p<Derive>;
	template<typename Derive> using multiplex_ioinfo_p = proxy_ioinfo_p<Derive>;
	template<typename Derive> using multiplex_read_map_p = proxy_read_map_p<Derive>;
	template<typename Derive> using multiplex_write_map_p = proxy_write_map_p<Derive>;

	// TODO: remove multiplex dependency
	template<typename ArchiveType> struct sub_archive: public proxy_archive<ArchiveType>
	{
		typedef proxy_archive<ArchiveType> base;
		sub_archive(): pos(0), first(0), last(0){}
		template<typename RealArchiveType>
		explicit sub_archive(RealArchiveType&& ar, ext_heap::handle h)
			: base(std::forward<RealArchiveType>(ar)),
			first(h.begin()), last(h.end())
		{}
		template<typename RealArchiveType>
		sub_archive(RealArchiveType&& ar, long_size_t first_, long_size_t last_)
			: base(std::forward<RealArchiveType>(ar))
			  , pos(first_), first(first_), last(last_)
		{
			XR_PRE_CONDITION(first <= last);
		}

        sub_archive(sub_archive&& rhs)
            : proxy_archive<ArchiveType>(std::forward<proxy_archive<ArchiveType> >(rhs))
            , pos(rhs.first), first(rhs.first), last(rhs.last)
        {}

        long_size_t pos;
		long_size_t first;
		long_size_t last;
	};

	template<typename Derive> struct sub_reader_p : reader
	{
		typedef typename reader::iterator iterator;

		virtual range<iterator> read(const range<iterator>& buf) {
			auto old_offset = underlying_<random>().offset();
            auto current = derive_().pos;
            underlying_<random>().seek(current);
            
			if (current >= derive_().first && current < derive_().last){
				auto min_size = std::min<long_size_t>(buf.size(), derive_().last - current);
				range<iterator> sub_buf(buf.begin(), buf.begin() + min_size);
				auto result =  underlying_<reader>().read(sub_buf);
                derive_().pos += min_size - result.size();
                underlying_<random>().seek(old_offset);
                return make_range(result.begin() , buf.end());
			}
			return buf;
		}

		virtual bool readable() const {
			auto current = derive_().pos;
			return underlying_<reader>().readable()
					&& current >= derive_().first
					&& current < derive_().last;
		}

		private:
		COMMON_IO_ADAPTOR_HELPER();
	};

	template<typename Derive> struct sub_writer_p : writer
	{
		typedef typename writer::iterator iterator;
		typedef typename writer::const_iterator const_iterator;

		virtual range<const_iterator> write(const range<const_iterator>& buf){
			auto current = underlying_<sequence>().offset();
			if (current >= derive_().first && current < derive_().last){
				auto min_size = std::min<long_size_t>(buf.size(), derive_().last - current);
				range<iterator> sub_buf(buf.begin(), buf.begin() + min_size);
				return underlying_<writer>().write(sub_buf);
			}
			return buf;
		}
		virtual bool writable() const{
			auto current = underlying_<sequence>().offset();
			return underlying_<writer>().writable()
					&& current >= derive_().first
					&& current < derive_().last;
		}
		virtual void sync(){
			underlying_<writer>().sync();
		}
		private:
		COMMON_IO_ADAPTOR_HELPER();
	};

	template<typename Derive> struct sub_ioctrl_p : ioctrl
	{
		virtual long_size_t truncate(long_size_t size){
			return unknow_size;
		}
	};

	template<typename Derive> struct sub_ioinfo_p : ioinfo
	{
		virtual long_size_t size() const{
			return derive_().last - derive_().first;
		}
		private:
		const Derive& derive_() const{ return static_cast<const Derive&>(*this);}
		Derive& derive_() { return static_cast<Derive&>(*this);}
	};

	template<typename Derive> struct sub_sequence_p : sequence
	{
		typedef typename Derive::archive_type archive_type;

		virtual long_size_t offset() const{
			return underlying_<sequence>().offset() - derive_().first;	//it's ok if current < first, rewind
		}
		virtual long_size_t size() const{
			return derive_().last - derive_().first;
		}
		private:
		COMMON_IO_ADAPTOR_HELPER();
	};

	template<typename Derive> struct sub_forward_p : forward
	{
		typedef typename Derive::archive_type archive_type;

		virtual long_size_t offset() const{
			return underlying_<forward>().offset() - derive_().first;	//it's ok if current < first, rewind
		}
		virtual long_size_t size() const{
			return derive_().last - derive_().first;
		}
		virtual long_size_t seek(long_size_t off){
			return underlying_<forward>().seek(derive_().first + off);
		}
		private:
		COMMON_IO_ADAPTOR_HELPER();
	};

	template<typename Derive> struct sub_random_p : random
	{
		virtual long_size_t offset() const{
			return underlying_<random>().offset() - derive_().first;	//it's ok if current < first, rewind
		}
		virtual long_size_t size() const{
			return derive_().last - derive_().first;
		}
		virtual long_size_t seek(long_size_t off){
			return underlying_<random>().seek(derive_().first + off);
		}
		private:
		COMMON_IO_ADAPTOR_HELPER();
	};

	template<typename Derive> struct sub_read_map_p : read_map
	{
		virtual iauto<read_view> view_rd(ext_heap::handle h){
			h = ext_heap::handle(h.begin() + derive_().first, h.end() + derive_().first);
			XR_PRE_CONDITION(h.begin() <= h.end()
					&& (long_size_t)h.begin() >= derive_().first
					&& (long_size_t)h.end() <= derive_().last);

			return underlying_<read_map>().view_rd(h);
		}
		virtual long_size_t size() const{
			return derive_().last - derive_().first;
		}
		private:
		COMMON_IO_ADAPTOR_HELPER();
	};

	template<typename Derive> struct sub_write_map_p : write_map
	{
		virtual iauto<write_view> view_wr(ext_heap::handle h){
			h = ext_heap::handle(h.begin() + derive_().first, h.end() + derive_().first);

			XR_PRE_CONDITION(h.begin() <= h.end()
					&& (long_size_t)h.begin() >= derive_().first
					&& (long_size_t)h.end() <= derive_().last);

			return underlying_<write_map>().view_wr(h);
		}
		virtual long_size_t size() const{
			return derive_().last - derive_().first;
		}
		virtual void sync() {
			return underlying_<write_map>().sync();
		}
		virtual long_size_t truncate(long_size_t /*s*/){
			return size();
		}
		private:
		COMMON_IO_ADAPTOR_HELPER();
	};

	template<typename ArchiveType>
	struct tail_archive : public proxy_archive<ArchiveType>
	{
		typedef proxy_archive<ArchiveType> base;
		tail_archive(): first(0){}
		template<typename RealArchiveType>
		tail_archive(RealArchiveType&& ar, long_size_t first_)
			: base(std::forward<RealArchiveType>(ar)) , first(first_)
		{
		}

        tail_archive(tail_archive&& rhs)
            : proxy_archive<ArchiveType>(std::forward<proxy_archive<ArchiveType> >(rhs))
            , first(rhs.first)
        {}

		long_size_t first;
	};
	template<typename Derive>
		using tail_reader_p = proxy_reader_p<Derive>;
	template<typename Derive>
		using tail_writer_p = proxy_writer_p<Derive>;

	template<typename Derive> struct tail_ioctrl_p : ioctrl
	{
		virtual long_size_t truncate(long_size_t size){
			return underlying_<ioctrl>().truncate(size + derive_().first);
		}
		private:
		COMMON_IO_ADAPTOR_HELPER();
	};

	template<typename Derive> struct tail_ioinfo_p : ioinfo
	{
		virtual long_size_t size() const{
			auto real_size = underlying_<ioinfo>().size();
			return real_size < derive_().first
				? 0
				: real_size - derive_().first;
		}
		private:
		COMMON_IO_ADAPTOR_HELPER();
	};

	template<typename Derive> struct tail_sequence_p : sequence
	{
		virtual long_size_t offset() const{
			return underlying_<sequence>().offset() - derive_().first;	//it's ok if current < first, rewind
		}
		virtual long_size_t size() const{
			auto real_size = underlying_<sequence>().size();
			return real_size < derive_().first
				? 0
				: real_size - derive_().first;
		}
		private:
		COMMON_IO_ADAPTOR_HELPER();
	};

	template<typename Derive> struct tail_forward_p : forward
	{
		virtual long_size_t offset() const{
			return underlying_<forward>().offset() - derive_().first;	//it's ok if current < first, rewind
		}
		virtual long_size_t size() const{
			auto real_size = underlying_<forward>().size();
			return real_size < derive_().first
				? 0
				: real_size - derive_().first;
		}
		virtual long_size_t seek(long_size_t off){
			return underlying_<forward>().seek(off + derive_().first);
		}
		private:
		COMMON_IO_ADAPTOR_HELPER();
	};

	template<typename Derive> struct tail_random_p : random
	{
		virtual long_size_t offset() const{
			return underlying_<random>().offset() - derive_().first;	//it's ok if current < first, rewind
		}
		virtual long_size_t size() const{
			auto real_size = underlying_<random>().size();
			return real_size < derive_().first
				? 0
				: real_size - derive_().first;
		}
		virtual long_size_t seek(long_size_t off){
			return underlying_<random>().seek(off + derive_().first);
		}
		private:
		COMMON_IO_ADAPTOR_HELPER();
	};

	template<typename Derive> struct tail_read_map_p : read_map
	{
		virtual iauto<read_view> view_rd(ext_heap::handle h){
			h = ext_heap::handle(h.begin() + derive_().first, h.end() + derive_().first);
			return underlying_<read_map>().view_rd(h);
		}
		virtual long_size_t size() const{
			auto real_size = underlying_<read_map>().size();
			return real_size < derive_().first
				? 0
				: real_size - derive_().first;
		}
		private:
		COMMON_IO_ADAPTOR_HELPER();
	};

	template<typename Derive> struct tail_write_map_p : write_map
	{
		virtual iauto<write_view> view_wr(ext_heap::handle h){
			h = ext_heap::handle(h.begin() + derive_().first, h.end() + derive_().first);

			return underlying_<write_map>().view_wr(h);
		}
		virtual long_size_t size() const{
			auto real_size = underlying_<write_map>().size();
			return real_size < derive_().first
				? 0
				: real_size - derive_().first;
		}
		virtual void sync() {
			underlying_<write_map>().sync();
		}
		virtual long_size_t truncate(long_size_t s){
			return underlying_<write_map>().truncate(derive_().first + s);
		}
		private:
		COMMON_IO_ADAPTOR_HELPER();
	};

	template<typename Derive> using stream_to_map_archive = proxy_archive<Derive>;

	struct stream_to_write_view : write_view
	{
		range<byte*> address() const {
			return to_range(cache.data());
		}
		~stream_to_write_view(){
			if (cache.size() != 0){
				write_back(*this);
			}
		}
		mutable mem_archive cache;
		std::function<void(stream_to_write_view& )> write_back;
	};

	template<typename Derive> struct stream_to_write_map_p : write_map
	{
		virtual iauto<write_view> view_wr(ext_heap::handle h){
			stream_to_write_view ret;
			underlying_<io::random>().seek(h.begin());
			iref<io::write_map> dest(ret.cache);
			copy_data(underlying_<reader>(), dest.get<write_map>(), h.size());
			ret.cache.truncate(h.size());
			ret.cache.seek(0);
			ret.write_back = [this, h](stream_to_write_view& view){
				underlying_<io::random>().seek(h.begin());
                iref<io::read_map> src(view.cache);
				copy_data(src.get<read_map>(), underlying_<writer>());
			};
			return iauto<write_view>(std::move(ret));
		}

		virtual long_size_t size() const{
			return underlying_<io::random>().size();
		}
		virtual void sync() {
			return underlying_<writer>().sync();
		}
		virtual long_size_t truncate(long_size_t s){
			return underlying_<ioctrl>().truncate(s);
		}
		private:
        //mem_archive cache;
		COMMON_IO_ADAPTOR_HELPER();
	};

	struct stream_to_read_map_view : read_view
	{
		virtual range<const byte*> address() const {
			return to_range(cache.data());
		}
		mem_archive cache;
	};
	template<typename Derive> struct stream_to_read_map_p : read_map
	{
		virtual iauto<read_view> view_rd(ext_heap::handle h){

			stream_to_read_map_view ret;
			underlying_<io::random>().seek(h.begin());
			iref<io::write_map> dest(ret.cache);
			copy_data(underlying_<reader>(), dest.get<io::write_map>(), h.size());
			ret.cache.seek(0);
			return iauto<read_view>(std::move(ret));
		}
		virtual long_size_t size() const{
			return underlying_<io::random>().size();
		}
		private:
		COMMON_IO_ADAPTOR_HELPER();
	};

	template<typename Derive> struct read_map_to_random_p : random {
		virtual long_size_t offset() const {
			return derive_().current;
		}
		virtual long_size_t size() const {
			return underlying_<read_map>().size();
		}
		virtual long_size_t seek(long_size_t current) {
			auto s = size();
			if (current < s)
				return (derive_().current = current);
			else
				return (derive_().current = s);
		}
		private:
		COMMON_IO_ADAPTOR_HELPER();

	};

	template<typename Derive> using map_to_stream_archive = multiplex_archive<Derive>;

	template<typename Derive> struct read_map_to_reader_p : reader
	{
		typedef typename reader::iterator iterator;

		virtual range<iterator> read(const range<iterator>& buf) {
			auto last = derive_().current + buf.size();
			last = std::min(last, underlying_<read_map>().size());
			ext_heap::handle h(derive_().current, last);
			auto view = underlying_<read_map>().view_rd(h);
			auto address = view.template get<io::read_view>().address();
			auto pos = std::copy(address.begin(), address.end(), buf.begin());
			derive_().current = last;
			return range<iterator>(pos, buf.end());
		}

		virtual bool readable() const {
			return derive_().current < underlying_<read_map>().size();
		}

		private:
		COMMON_IO_ADAPTOR_HELPER();
	};

	template<typename Derive> struct write_map_to_writer_p : writer
	{
		virtual range<const_iterator> write(const range<const_iterator>& r){
			auto last = derive_().current + r.size();
			ext_heap::handle h(derive_().current, last);
			auto view = underlying_<write_map>().view_wr(h);
			auto address = view.address();
			std::copy(r.begin(), r.end(), address.begin());
			derive_().current = last;
			return range<iterator>(r.end(), r.end());
		}

		virtual bool writable() const {
			return true;
		}

		virtual void sync(){
			underlying_<write_map>().sync();
		}
		private:
		COMMON_IO_ADAPTOR_HELPER();
	};
	template<typename Derive> struct write_map_to_random_p : random {
		virtual long_size_t offset() const {
			return derive_().current;
		}
		virtual long_size_t size() const {
			return underlying_<write_map>().size();
		}
		virtual long_size_t seek(long_size_t current) {
			return (derive_().current = current);
		}
		private:
		COMMON_IO_ADAPTOR_HELPER();
	};


#undef COMMON_IO_ADAPTOR_HELPER
}}
#endif //end XR_COMMON_ARCHIVE_ADAPTOR_H__

