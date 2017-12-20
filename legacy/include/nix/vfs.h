#ifndef XR_XIRANG2_VFS_H__
#define XR_XIRANG2_VFS_H__

#include <xirang2/io.h>
#include <xirang2/fsutility.h>
#include <xirang2/mpl.h>
#include <xirang2/type/object.h>

// STD
#include <memory>

namespace xirang2 { namespace vfs{ namespace detail{
	template<typename T> struct interface_mask;

	template<> struct interface_mask<io::sequence>{
		static const unsigned long long value = 1;
	};
	template<> struct interface_mask<io::forward>{
		static const unsigned long long value = 3;
	}; 
	template<> struct interface_mask<io::random>{ 
		static const unsigned long long value = 7; 
	}; 
	template<> struct interface_mask<io::reader>{ 
		static const unsigned long long value = 1 << 3;
	}; 
	template<> struct interface_mask<io::writer>{
		static const unsigned long long value = 1 << 4; 
	}; 
	template<> struct interface_mask<io::ioctrl>{
		static const unsigned long long value = 1 << 5; 
	}; 
	template<> struct interface_mask<io::options>{
		static const unsigned long long value = 1 << 6;
	}; 
	template<> struct interface_mask<io::read_view>{
		static const unsigned long long value = 1 << 7;
	}; 
	template<> struct interface_mask<io::write_view>{
		static const unsigned long long value = 1 << 8;
	};
	template<> struct interface_mask<io::read_map>{
		static const unsigned long long value = 1 << 9;
	}; 
	template<> struct interface_mask<io::write_map>{
		static const unsigned long long value = 1 << 10;
	};

	template<typename I, typename... Interfaces> struct get_mask{
		static const unsigned long long value = 
			interface_mask<I>::value | get_mask<Interfaces...>::value;
	};
	template<typename T> struct get_mask<T>{
		static const unsigned long long value = interface_mask<T>::value;
	};
}

XR_EXCEPTION_TYPE(unsupport_interface);

namespace private_{ 
	template<typename... Interfaces> struct map_to_iref;
	template<typename... Interfaces> struct map_to_iref<mpl::seq<Interfaces...>>{ 
		typedef iref<Interfaces...> type;
	};

	template<typename T, typename U> struct less_interface{
		static const bool value = 
			detail::interface_mask<T>::value < detail::interface_mask<U>::value;
	};

	template<typename... Interfaces> struct sorted_iref{
		typedef mpl::seq<Interfaces...> seq;
		typedef typename mpl::sort<seq, less_interface>::type sorted_seq;
		typedef typename map_to_iref<sorted_seq>::type type;
	};

	template<typename... Interfaces> struct map_to_iauto;
	template<typename... Interfaces> struct map_to_iauto<mpl::seq<Interfaces...>>{
		typedef iauto<Interfaces...> type;
	};

	template<typename... Interfaces> struct sorted_iauto{
		typedef mpl::seq<Interfaces...> seq;
		typedef typename mpl::sort<seq, less_interface>::type sorted_seq;
		typedef typename map_to_iauto<sorted_seq>::type type;
	};

	template<typename... Interfaces> struct copy_interface_helper;

	template<> struct copy_interface_helper<mpl::seq<>> {
		template<typename IRef> static void** copy(unsigned long long 
				, void** ret, const IRef& , void* )
		{
			return ret; 
		} 
	}; 
	template<typename T, typename... Interfaces> struct copy_interface_helper<mpl::seq<T, Interfaces...>> { 
		template<typename IRef> static void** copy(unsigned long long mask, 
				void** ret, const IRef& ref, void* this_){ 
			if (mask & detail::interface_mask<T>::value) {
				*ret++ = *(void**)&ref.template get<T>(); *ret++ = this_;
			}
			return copy_interface_helper<mpl::seq<Interfaces...> >::template copy(mask, ret, ref, this_);
		} 
	}; 
}
template<typename... Interfaces> struct copy_interface{ 
	template<typename CoClass> static void** apply(unsigned long long mask,
			void** ret, CoClass& ref, void* this_){
		if((mask & detail::get_mask<Interfaces...>::value) != mask)
			XR_THROW(unsupport_interface);

		typedef typename private_::sorted_iref<Interfaces...>::sorted_seq seq;
		return private_::copy_interface_helper<seq>::template copy(mask, ret, ref, this_); 
	}
};


class IVfs; 
class RootFs;
class RootFsImp;

using fs::fs_error;
using fs::file_state;
using io::io_option;

struct MountInfo { 
	file_path path; 
	IVfs* fs;
}; 

struct XR_API MetadataValue{ 
	file_path file_type;

	bool empty() const; 
	type::CommonObject metadata() const;
	void clean_metadata();
	void take_metadata(type::CommonObject from);
	void assign_metadata(type::ConstCommonObject from);

	MetadataValue();
	explicit MetadataValue(file_path path, type::CommonObject obj);
	MetadataValue(const MetadataValue& rhs);
	MetadataValue(MetadataValue&& rhs);
	MetadataValue& operator=(MetadataValue rhs);
	~MetadataValue();
	private:
	type::CommonObject m_metadata;
};

XR_API void save(io::writer& wr, const MetadataValue& v);
XR_API void load(io::reader& rd, MetadataValue& v, type::Xirang& xr_, type::Namespace temp_root);

struct XR_API VfsState : public fs::fstate {
	IVfs* owner_fs;

	MetadataValue metadata;

	VfsState();
	explicit VfsState(IVfs* owner, file_state state, file_path p = file_path(), bool withTime = true);
	~VfsState();

	VfsState(const VfsState& );
	VfsState(VfsState&& );
	VfsState& operator=(const VfsState& );
	VfsState& operator=(VfsState&& );
	VfsState& assign(const VfsState& , io_option option);	// can be ao_default or ao_metadata 
};

typedef ForwardRangeT<const_itr_traits<MountInfo> > VfsRange;
typedef ForwardRangeT<const_itr_traits<VfsState> > VfsNodeRange;
template<typename... Interfaces> typename 
private_::sorted_iauto<Interfaces...>::type create(IVfs& fs, sub_file_path path, int flag);

class XR_INTERFACE IRepository;

/// \notes 1. all vfs modifications will return er_ok or not null archive_ptr if succeeded.  
///     2. the path or file name must be xirang2 style, separated by "/".  
///     3. supported file name character set is depends on vfs implementation.  
///     4. the returned fs_error is implementation defined, if user just depends on IVfs, 
///			should not assume which error code will be returned, except return er_ok case.  
///			but, for a known vfs imp, the error code should be determinated.  
///
class XR_API IVfs { 
	public:
	/// common operations of dir and file 
	/// \pre path must not end with '/'.  
	/// \pre !absolute(path) 
	/// \notes the result of removeing a opended file is depends on implementation capability..
	fs_error remove(sub_file_path path) { return do_remove(path);}

	/// create dir 
	/// \pre path must not end with '/'.  
	/// \pre !absolute(path) 
	/// \notes if the parent of path is not exist, failed.
	/// \notes the result depends on implementation capability.
	fs_error createDir(sub_file_path path) { return do_createDir(path); }

	template<typename... Interfaces> typename
		private_::sorted_iauto<Interfaces...>::type create(sub_file_path
				path, int flag){ 
			return
			xirang2::vfs::create<Interfaces...>(*this, path, flag);
		}

	/// create or open file 
	/// \param compound flag of io::archive_mode 
	/// \param flag one of io::open_flag 
	/// \pre path must not end with '/'.
	/// \pre !absolute(path) 
	/// \notes the capability of returned archive_ptr must greater than or equal to given mode.  
	///         but if user code just depends on IVfs, should not assume the addational capability 
	///			not specified by given mode 
	/// \notes if the parent of path is not exist, failed.  
	/// \notes the result depends on implementation capability.
	void** raw_create(unsigned long long mask,
			void** ret, unique_ptr<void>& owner, sub_file_path path, int flag) {
		return do_create(mask, ret, owner, path, flag);
	}
	/// copy file via file path
	/// \pre !absolute(to)
	/// \pre !absolute(from) || mounted()
	/// if from and to in same fs, it may have a more effective implementation
	/// \notes copy to self is imp defined.
	fs_error copy(sub_file_path from, sub_file_path to){ return do_copy(from, to);}

	/// move file via file path
	/// \pre !absolute(to)
	/// \pre !absolute(from) || mounted()
	/// if from and to in same fs, it may have a more effective implementation
	fs_error move(sub_file_path from, sub_file_path to){ return do_move(from, to);}

	/// truncate a file with given size
	/// \pre path must not end with '/'.
	/// \pre !absolute(to)
	fs_error truncate(sub_file_path path, long_size_t s){ return do_truncate(path, s);}

	/// flush buffered data to underly media. implementation defined.
	void sync(){ return do_sync();}

	/// query
	const string& resource() const{ return do_resource();}

	/// volume
	/// if !mounted, return null
	RootFs* root() const{ return do_root();}

	/// \post mounted() && root() || !mounted() && !root()
	bool mounted() const{ return do_mounted();}

	/// \return mounted() ? absolute() : empty()
	file_path mountPoint() const { return do_mountPoint();}

	/// \pre path must not end with '/'.
	/// \pre !absolute(path)
	/// \return returned path in VfsState is file name, not full path.
	VfsNodeRange children(sub_file_path path, io_option option = io_option::ao_default) const { return do_children(path, option);}

	/// \pre path must not end with '/'.
	/// \pre !absolute(path)
	/// \return returned path in VfsState is full path in this vfs. it's different from children()
	VfsState state(sub_file_path path, io_option option = io_option::ao_default) const { return do_state(path, option);}

	/// get the option with given id and data
	/// if failed, return a empty any, otherwise depends on the input value.
	/// all options are implementation defined
	any getopt(int id, const any & optdata = any() ) const { return do_getopt(id, optdata);}

	/// set the option with given id, option data
	/// if failed, return a empty any, otherwise depends the input value.
	/// all options are implementation defined
	any setopt(int id, const any & optdata,  const any & indata= any()) { return do_setopt(id, optdata, indata);}

	void setRoot(RootFs* r) { do_setRoot(r);}
	virtual ~IVfs();

	private:
	virtual fs_error do_remove(sub_file_path path) = 0;
	virtual fs_error do_createDir(sub_file_path path) = 0;
	virtual fs_error do_copy(sub_file_path from, sub_file_path to) = 0;
	virtual fs_error do_move(sub_file_path from, sub_file_path to) = 0;
	virtual fs_error do_truncate(sub_file_path path, long_size_t s) = 0;
	virtual void do_sync() = 0;
	virtual const string& do_resource() const = 0;
	virtual RootFs* do_root() const = 0;
	virtual bool do_mounted() const = 0;
	virtual file_path do_mountPoint() const = 0;
	virtual VfsNodeRange do_children(sub_file_path path, io_option option) const = 0;
	virtual VfsState do_state(sub_file_path path, io_option option) const = 0;
	virtual any do_getopt(int id, const any & optdata) const ;
	virtual any do_setopt(int id, const any & optdata,  const any & indata);

	/// create or open file
	/// \param compound flag of io::archive_mode
	/// \param flag one of io::open_flag
	/// \pre path must not end with '/'.
	/// \pre !absolute(path)
	/// \notes the capability of returned archive_ptr must greater than or equal to given mode.
	///         but if user code just depends on IVfs, should not assume the addational capability not specified by given mode
	/// \notes if the parent of path is not exist, failed.
	/// \notes the result depends on implementation capability.
	virtual void** do_create(unsigned long long mask,
			void** ret, unique_ptr<void>& owner, sub_file_path path, int flag) = 0;

	// if r == null, means unmount, used by RootFs only
	virtual void do_setRoot(RootFs* r) = 0;

	friend class RootFsImp;
};
template<typename... Interfaces> typename private_::sorted_iauto<Interfaces...>::type
create(IVfs& vfs, sub_file_path path, int flag){
	typedef typename private_::sorted_iauto<Interfaces...>::type interface_type;
	const auto mask = detail::get_mask<Interfaces...>::value;
	interface_type ret;
	target_holder<void>* holder = &ret;

	void** last = 0;
	if (flag & int(io::of_nothrow)){
		try{
			last = vfs.raw_create(mask, (void**)(holder + 1), ret.target_ptr, path, (flag & (~int(io::of_nothrow))));
		}
		catch(xirang2::exception& ){
		}
		catch(std::exception&){
		}
	}
	else{
		last = vfs.raw_create(mask, (void**)(holder + 1), ret.target_ptr, path, flag);
	}

	if (last){
		XR_PRE_CONDITION(last == (void**)&ret.target_ptr);
		holder->target = ret.target_ptr.get();
	}
	return std::move(ret);
}

template<typename CoClass> struct IVfsCo: public IVfs
{
	virtual fs_error do_remove(sub_file_path path) {
		return get_cobj<CoClass>(this).remove(path);
	}

	virtual fs_error do_createDir(sub_file_path path){
		return get_cobj<CoClass>(this).createDir(path);
	}

	virtual fs_error do_copy(sub_file_path from, sub_file_path to){
		return get_cobj<CoClass>(this).copy(from, to);
	}
	virtual fs_error do_move(sub_file_path from, sub_file_path to){
		return get_cobj<CoClass>(this).move(from, to);
	}

	virtual fs_error do_truncate(sub_file_path path, long_size_t s){
		return get_cobj<CoClass>(this).truncate(path, s);
	}

	virtual void do_sync(){
		return get_cobj<CoClass>(this).sync();
	}

	virtual const string& do_resource() const{
		return get_cobj<CoClass>(this).resource();
	}

	virtual RootFs* do_root() const{
		return get_cobj<CoClass>(this).root();
	}

	virtual bool do_mounted() const{
		return get_cobj<CoClass>(this).mounted();
	}

	virtual file_path do_mountPoint() const{
		return get_cobj<CoClass>(this).mountPoint();
	}

	virtual VfsNodeRange do_children(sub_file_path path, io_option option) const{
		return get_cobj<CoClass>(this).children(path, option);
	}

	virtual VfsState do_state(sub_file_path path, io_option option) const{
		return get_cobj<CoClass>(this).state(path, option);
	}

	virtual any do_getopt(int id, const any & optdata) const{
		return get_cobj<CoClass>(this).getopt(id, optdata);
	}

	virtual any do_setopt(int id, const any & optdata, const any & indata){
		return get_cobj<CoClass>(this).setopt(id, optdata, indata);
	}

	virtual void** do_create(unsigned long long mask,
			void** base, unique_ptr<void>& owner, sub_file_path path, int flag) {
		return get_cobj<CoClass>(this).do_create(mask, base, owner, path, flag);
	}

	virtual void do_setRoot(RootFs* r){
		return get_cobj<CoClass>(this).setRoot(r);
	}
};

template<typename CoClass>
IVfsCo<CoClass> get_interface_map(IVfs*, CoClass*);

class XR_API RootFs
{
	public:
		explicit RootFs(const string& res);
		~RootFs();

		// \pre dir must be absolute name
		fs_error mount(sub_file_path dir, IVfs& vfs);
		fs_error mount(sub_file_path dir, xirang2::unique_ptr<IVfs>& vfs);
		fs_error unmount(sub_file_path dir);

		void sync();

		// query
		const string& resource() const;

		// volume
		// return absolute name
		file_path mountPoint(const IVfs& p) const;
		IVfs* mountPoint(sub_file_path p) const;

		VfsRange mountedFS() const;
		VfsState locate(sub_file_path path) const;

		// return true if path contains mount pointer in direct/indirect subdir, excluding self.
		bool containMountPoint(sub_file_path path) const;


		//Methods of IVfs
		fs_error remove(sub_file_path path);
		fs_error createDir(sub_file_path path);
		fs_error copy(sub_file_path from, sub_file_path to) ;
		fs_error move(sub_file_path from, sub_file_path to);
		fs_error truncate(sub_file_path path, long_size_t s) ;
		VfsNodeRange children(sub_file_path path, io_option option = io_option::ao_default) const ;
		VfsState state(sub_file_path path, io_option option = io_option::ao_default) const ;
		void** raw_create(unsigned long long mask,
				void** ret, unique_ptr<void>& owner, sub_file_path path, int flag);

		template<typename... Interfaces> typename private_::sorted_iauto<Interfaces...>::type
			create(sub_file_path path, int flag){
				typedef typename private_::sorted_iauto<Interfaces...>::type interface_type;
				const auto mask = detail::get_mask<Interfaces...>::value;
				interface_type ret;
				target_holder<void>* holder = &ret;
				void** last = 0;
				if (flag & int(io::of_nothrow)){
					try{
						last = this->raw_create(mask, (void**)(holder + 1), ret.target_ptr, path, (flag & (~int(io::of_nothrow))));
					}
					catch(xirang2::exception& ){
					}
					catch(std::exception&){
					}
				}
				else{
					last = this->raw_create(mask, (void**)(holder + 1), ret.target_ptr, path, flag);
				}

				XR_PRE_CONDITION(last == 0 || last == (void**)&ret.target_ptr);
				unuse(last);
				holder->target = ret.target_ptr.get();
				return std::move(ret);
			}
	private:
		RootFsImp* m_imp;
};

template<typename... Interfaces> typename private_::sorted_iauto<Interfaces...>::type
temp_file(IVfs& vfs, sub_file_path template_ , sub_file_path parent_dir
		, int flag = io::of_remove_on_close, file_path* path = 0){
	XR_PRE_CONDITION(flag == 0 ||  flag  == io::of_remove_on_close);
	XR_PRE_CONDITION(!parent_dir.is_root());
	flag |= io::of_create;

	if (vfs.state(parent_dir).state != fs::st_dir)
		XR_THROW(fs::not_found_exception)("failed to locate the temp directory:")(parent_dir.str());

	const int max_try = 10;
	for(int i = 0; i < max_try ; ++i)
	{
		auto fpath = parent_dir / fs::private_::gen_temp_name(template_);

		if (vfs.state(fpath).state != fs::st_not_found)
			continue;
		auto ret = create<Interfaces...>(vfs, fpath, flag);
		if (path)
			*path = fpath;
		return std::move(ret);
	}

	XR_THROW(fs::open_failed_exception)("failed to create temp file in directory:")(parent_dir.str());
}

XR_API file_path temp_dir(IVfs& vfs, sub_file_path template_, sub_file_path parent_dir);
XR_API fs_error recursive_remove(IVfs& vfs, sub_file_path path);
XR_API fs_error recursive_create_dir(IVfs& vfs, sub_file_path path);

template<typename... Interfaces> typename private_::sorted_iauto<Interfaces...>::type
recursive_create(IVfs& vfs, sub_file_path path, int flag){
	fs_error err = recursive_create_dir(vfs, path.parent());
	if (err != fs::er_ok)
		XR_THROW(fs::open_failed_exception)("failed to create the parent directory:")(path.str());

	return create<Interfaces...>(vfs, path, flag);
}

}}

#endif //end XR_XIRANG2_VFS_H__

