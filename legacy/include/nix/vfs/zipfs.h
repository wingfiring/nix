#ifndef XIRANG2_ZIP_VFS_H__
#define XIRANG2_ZIP_VFS_H__

#include <xirang2/vfs.h>
#include <xirang2/io.h>

namespace xirang2{ namespace vfs{

	class ZipFsImp;
	class XR_API ZipFs : public IVfs
	{
	public:
        explicit ZipFs(const iref<io::read_map, io::write_map>& ar, IVfs* cache, const string& resource = string());
        explicit ZipFs(iauto<io::read_map, io::write_map> ar, IVfs* cache, const string& resource = string());
        explicit ZipFs(const iref<io::read_map>& ar, IVfs* cache, const string& resource = string());
        explicit ZipFs(iauto<io::read_map> ar, IVfs* cache, const string& resource = string());

		~ZipFs();

		// common operations of dir and file
		// \pre !absolute(path)
		virtual fs_error do_remove(sub_file_path path);

		// dir operations
		// \pre !absolute(path)
		virtual fs_error do_createDir(sub_file_path path);

		// \pre !absolute(to)
		// if from and to in same fs, it may have a more effective implementation
		// otherwise, from should be a
		virtual fs_error do_copy(sub_file_path from, sub_file_path to);

		virtual fs_error do_move(sub_file_path from, sub_file_path to);

		virtual fs_error do_truncate(sub_file_path path, long_size_t s);

		virtual void do_sync();

		// query
		virtual const string& do_resource() const;

		// volume
		// if !mounted, return null
		virtual RootFs* do_root() const;

		// \post mounted() && root() || !mounted() && !root()
		virtual bool do_mounted() const;

		// \return mounted() ? absolute() : empty() 
		virtual file_path do_mountPoint() const;

		// \pre !absolute(path)
		virtual VfsNodeRange do_children(sub_file_path path, io_option option) const;

		// \pre !absolute(path)
		virtual VfsState do_state(sub_file_path path, io_option option) const;

		virtual void** do_create(unsigned long long mask,
				void** base, unique_ptr<void>& owner, sub_file_path path, int flag);

	private:
		// if r == null, means unmount
		virtual void do_setRoot(RootFs* r);

		unique_ptr<ZipFsImp> m_imp;

		ZipFs(const ZipFs&) = delete;
		ZipFs& operator=(const ZipFs&) = delete;

	};

}}

#endif //end XIRANG2_ZIP_VFS_H__

