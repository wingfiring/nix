#ifndef XR_XIRANG2_INMEMORY_FS_H__
#define XR_XIRANG2_INMEMORY_FS_H__

#include <xirang2/vfs.h>
#include <xirang2/io/memory.h>

namespace xirang2{ namespace vfs{

	class InMemoryImp;
	class XR_API InMemory : public IVfs
	{
	public:

        explicit InMemory(const string& resource = string());

		~InMemory();

		// common operations of dir and file
		// \pre !absolute(path)
		virtual fs_error do_remove(sub_file_path path);

		// dir operations
		// \pre !absolute(path)
		virtual fs_error do_createDir(sub_file_path path);

		// file operations
		io::buffer_io writeOpen(sub_file_path path, int flag);
		io::buffer_in readOpen(sub_file_path path);

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

        virtual any do_getopt(int id, const any & optdata) const ;
        virtual any do_setopt(int id, const any & optdata, const any & indata);

		virtual void** do_create(unsigned long long mask,
				void** base, unique_ptr<void>& owner, sub_file_path path, int flag);

		InMemory(const InMemory&) = delete;
		InMemory& operator=(const InMemory&) = delete;
	private:
		// if r == null, means unmount
		virtual void do_setRoot(RootFs* r);
		InMemoryImp* m_imp;
	};

}}
#endif //end XR_XIRANG2_INMEMORY_FS_H__



