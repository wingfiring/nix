#ifndef XR_XIRANG2_VFS_SUBVFS_H__
#define XR_XIRANG2_VFS_SUBVFS_H__

#include <xirang2/vfs.h>

namespace xirang2{ namespace vfs{

	class XR_API SubVfs : public IVfs
	{
	public:

		explicit SubVfs(IVfs& parent, sub_file_path dir);

		~SubVfs();

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

        IVfs& parentFs() const;

		virtual void** do_create(unsigned long long mask,
				void** ret, unique_ptr<void>& owner, sub_file_path path, int flag);
	private:
		// if r == null, means unmount
		virtual void do_setRoot(RootFs* r);

        RootFs* m_root;
        IVfs& parent;
        file_path m_resource;
    

        SubVfs(const SubVfs&); //disable
        SubVfs& operator=(const SubVfs&);//disable

	};

}}

#endif //end XR_XIRANG2_VFS_SUBVFS_H__

